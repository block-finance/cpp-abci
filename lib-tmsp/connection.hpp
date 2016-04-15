#pragma once


#include <tmsp/types.pb.h>
#include <boost/asio.hpp>
#include <memory>


namespace tmsp
{


typedef std::array<char, 8192> buffer_type;


class connection_type : public std::enable_shared_from_this<connection_type>
{
public:
	connection_type(boost::asio::ip::tcp::socket& socket);

	void start();

protected:
	boost::asio::ip::tcp::socket socket_;
	buffer_type read_buffer_;
	buffer_type write_buffer_;
	buffer_type::pointer read_head_;
	buffer_type::pointer read_tail_;
	buffer_type::pointer write_head_;
	buffer_type::pointer write_tail_;

	void async_read_();
	void async_write_();
	void try_advance_read_buffer_();
	bool prepare_response_(types::Request const& request, types::Response& response);
	bool write_message_(google::protobuf::MessageLite const& message);
};


}	// namespace tmsp
