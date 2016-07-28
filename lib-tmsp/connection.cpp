#include "connection.hpp"
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <tmsp/types.pb.cc>


namespace tmsp
{


namespace wire
{


/*
	Original tendermint implementation:

	func ReadVarint(r io.Reader, n *int, err *error) int {
		var size = ReadUint8(r, n, err)
		var negate = false
		if (size >> 4) == 0xF {
			negate = true
			size = size & 0x0F
		}
		if size > 8 {
			setFirstErr(err, errors.New("Varint overflow"))
			return 0
		}
		if size == 0 {
			if negate {
				setFirstErr(err, errors.New("Varint does not allow negative zero"))
			}
			return 0
		}
		buf := make([]byte, 8)
		ReadFull(buf[(8-size):], r, n, err)
		var i = int(binary.BigEndian.Uint64(buf))
		if negate {
			return -i
		} else {
			return i
		}
	}
*/

template<typename Integer>
bool read_variable_integer(buffer_type::pointer& iter, buffer_type::pointer end, Integer& result)
{
	auto size(*iter & char(0x0F));
	auto const negate(*iter & char(0xF0));

	if(size == 0 && negate)
	{
		BOOST_THROW_EXCEPTION(std::range_error("varint does not allow negative zero"));
	}
	else if(size > 8)
	{
		BOOST_THROW_EXCEPTION(std::range_error("varint overflow"));
	}
	else if(++ iter + size > end)
	{
		return false;
	}

	result = 0;
	for(; size; -- size, ++ iter)
	{
		result <<= 8;
		result += static_cast<std::uint8_t>(*iter);
	}

	if(negate)
	{
		result = - result;
	}

	return true;
}


/*
	Original tendermint implementation:

	func WriteVarint(i int, w io.Writer, n *int, err *error) {
		var negate = false
		if i < 0 {
			negate = true
			i = -i
		}
		var size = uvarintSize(uint64(i))
		if negate {
			// e.g. 0xF1 for a single negative byte
			WriteUint8(uint8(size+0xF0), w, n, err)
		} else {
			WriteUint8(uint8(size), w, n, err)
		}
		if size > 0 {
			buf := make([]byte, 8)
			binary.BigEndian.PutUint64(buf, uint64(i))
			WriteTo(buf[(8-size):], w, n, err)
		}
	}
*/

template<typename Integer>
bool write_variable_integer(buffer_type::pointer& iter, buffer_type::pointer end, Integer value)
{
	if(iter == end)
	{
		return false;
	}

	auto& size(*iter);
	if(value < 0)
	{
		size = char(0xF0);
		value = - value;
	}
	else
	{
		size = char(0x00);
	}

	for(++ iter; value; ++ size, ++ iter)
	{
		if(iter == end)
		{
			return false;
		}

		*iter = static_cast<std::uint8_t>(value);
		value >>= 8;
	}

	return true;
}


}	// namespace wire


connection_type::connection_type(boost::asio::ip::tcp::socket socket, application_ptr_type const& application) :
	socket_(std::move(socket)),
	read_head_(read_buffer_.data()),
	read_tail_(read_head_),
	write_head_(write_buffer_.data()),
	write_tail_(write_head_),
	application_(application)
{
}


void connection_type::start()
{
	async_read_();
}


void connection_type::async_read_()
{
	auto const self(shared_from_this());
	auto const remaining(read_buffer_.data() + read_buffer_.size() - read_tail_);
	boost::asio::async_read(socket_, boost::asio::buffer(read_tail_, remaining), boost::asio::transfer_at_least(1),
		[self, this](boost::system::error_code const& error, std::size_t const& transferred)
		{
			if(error)
			{
				return;
			}

			read_tail_ += transferred;
			try_advance_read_buffer_();
			async_read_();
		});
}


void connection_type::async_write_()
{
	auto const self(shared_from_this());
	auto const remaining(write_tail_ - write_head_);
	if(remaining)
	{
		boost::asio::async_write(socket_, boost::asio::buffer(write_head_, remaining), boost::asio::transfer_all(),
			[self, this](boost::system::error_code const& error, std::size_t const& transferred)
			{
				if(error)
				{
					BOOST_LOG_TRIVIAL(error) << socket_.remote_endpoint() << ": Write error: " << error;
					return;
				}

				async_write_();
			});
		
		write_head_ = write_tail_;
	}
	else
	{
		write_head_ = write_buffer_.data();
		write_tail_ = write_head_;

		// Now that the write buffer is empty, see if there are any unprocessed requests available
		try_advance_read_buffer_();
	}
}


void connection_type::try_advance_read_buffer_()
{
	while(read_head_ != read_tail_)
	{
		auto iter(read_head_);
		std::int32_t message_size(0);
		if(!wire::read_variable_integer(iter, read_tail_, message_size))
		{
			break;
		}

		if(message_size <= 0)
		{
			BOOST_THROW_EXCEPTION(std::range_error("invalid message size"));
		}

		if(iter + message_size > read_tail_)
		{
			// Buffer does not contain the full message
			break;
		}

		types::Request request;
		request.ParseFromArray(iter, message_size);
		if(!request.IsInitialized())
		{
			BOOST_THROW_EXCEPTION(std::runtime_error("unable to parse request"));
		}

		// Only advance read head (i.e. treat request as completed) if a response could be prepared and written
		types::Response response;
		if(prepare_response_(request, response))
		{
			if(!write_message_(response))
			{
				BOOST_LOG_TRIVIAL(warning) << "Unable to write response - buffer full";
				break;
			}
		}
		read_head_ = iter + message_size;
	}

	if(read_head_ == read_tail_)
	{
		// All data consumed
		read_head_ = read_buffer_.data();
		read_tail_ = read_head_;
	}
	else
	{
		// Copy remaining data to front of buffer
		read_tail_ = std::copy(read_head_, read_tail_, read_buffer_.data());
		read_head_ = read_buffer_.data();
	}
}


bool connection_type::prepare_response_(types::Request const& request, types::Response& response)
{
	if(request.has_echo())
	{
		response.mutable_echo()->set_message(request.echo().message());
		return true;
	}
	else if(request.has_flush())
	{
		response.mutable_flush();
		return true;
	}
	else if(request.has_info())
	{
		application_->info(*response.mutable_info()->mutable_info());
		return true;
	}
	else if(request.has_set_option())
	{
		application_->set_option(request.set_option().key(), request.set_option().value(), *response.mutable_set_option()->mutable_log());
		return true;
	}
	else if(request.has_query())
	{
		auto const query_response(response.mutable_query());
		query_response->set_code(application_->query(request.query().query(), *query_response->mutable_data(), *query_response->mutable_log()));
		return true;
	}
	else if(request.has_begin_block())
	{
		application_->begin_block(request.begin_block().height());
		return false;
	}
	else if(request.has_end_block())
	{
		// TODO: Implement validator diffs
		response.mutable_end_block();
		return true;
	}
	else if(request.has_check_tx())
	{
		auto const check_tx_response(response.mutable_check_tx());
		check_tx_response->set_code(application_->check_tx(request.check_tx().tx(), *check_tx_response->mutable_data(), *check_tx_response->mutable_log()));
		return true;
	}
	else if(request.has_append_tx())
	{
		auto const append_tx_response(response.mutable_append_tx());
		append_tx_response->set_code(application_->append_tx(request.check_tx().tx(), *append_tx_response->mutable_data(), *append_tx_response->mutable_log()));
		return true;
	}
	else if(request.has_commit())
	{
		auto const commit_response(response.mutable_commit());
		commit_response->set_code(application_->commit(*commit_response->mutable_data(), *commit_response->mutable_log()));
		return true;
	}

	BOOST_THROW_EXCEPTION(std::logic_error("unsupported TMSP request: " + request.DebugString()));
}


bool connection_type::write_message_(google::protobuf::MessageLite const& message)
{
	auto iter(write_tail_);
	if(!wire::write_variable_integer(iter, write_buffer_.data() + write_buffer_.size(), message.ByteSize()))
	{
		// Not enough room for message size
		return false;
	}

	if(write_tail_ + message.ByteSize() > write_buffer_.data() + write_buffer_.size())
	{
		// Not enough room for message content
		return false;
	}

	if(!message.SerializeToArray(iter, message.ByteSize()))
	{
		BOOST_THROW_EXCEPTION(std::runtime_error("unable to serialize message"));
	}
	write_tail_ = iter + message.ByteSize();

	// Start async write if the write head is at the start of the buffer
	if(write_head_ == write_buffer_.data())
	{
		async_write_();
	}

	return true;
}


}	// namespace tmsp