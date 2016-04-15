#include "counter_application.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/format.hpp>


namespace counter
{


application_type::application_type() : hash_count_(0), tx_count_(0), serial_(false)
{
}


/*
	Original tendermint implementation:

	func (app *CounterApplication) Info() string {
		return Fmt("hashes:%v, txs:%v", app.hashCount, app.txCount)
	}
*/

void application_type::info(std::string& out)
{
	std::unique_lock<std::mutex> lock(mutex_);
	out = boost::str(boost::format("hashes: %1%, txs: %2%") % hash_count_ % tx_count_);
}


/*
	Original tendermint implementation:
	
	func (app *CounterApplication) SetOption(key string, value string) (log string) {
		if key == "serial" && value == "on" {
			app.serial = true
		}
		return ""
	}
*/

void application_type::set_option(std::string const& key, std::string const& value, std::string& log)
{
	if(boost::iequals(key, "serial") && boost::iequals(value, "on"))
	{
		std::unique_lock<std::mutex> lock(mutex_);
		serial_ = true;
	}
}


/*
	Original tendermint implementation:

	func (app *CounterApplication) Query(query []byte) types.Result {
		return types.NewResultOK(nil, fmt.Sprintf("Query is not supported"))
	}
*/

application_type::result_type application_type::query(std::string const& in, std::string& out, std::string& log)
{
	log = "Query is not supported";
	return result_type::OK;
}


void application_type::begin_block(std::uint64_t const& height)
{
}


application_type::result_type application_type::end_block(std::uint64_t const& height)
{
	return result_type::OK;	
}


/*
	Original tendermint implementation:

	func (app *CounterApplication) CheckTx(tx []byte) types.Result {
		if app.serial {
			tx8 := make([]byte, 8)
			copy(tx8[len(tx8)-len(tx):], tx)
			txValue := binary.BigEndian.Uint64(tx8)
			if txValue < uint64(app.txCount) {
				return types.Result{
					Code: types.CodeType_BadNonce,
					Data: nil,
					Log:  fmt.Sprintf("Invalid nonce. Expected >= %v, got %v", app.txCount, txValue),
				}
			}
		}
		return types.OK
	}
*/
	
application_type::result_type application_type::check_tx(std::string const& in, std::string& out, std::string& log)
{
	std::unique_lock<std::mutex> lock(mutex_);
	if(serial_)
	{
		std::uint64_t tx(0);
		for(auto const& c: in)
		{
			tx <<= 8;
			tx += static_cast<std::uint8_t>(c);
		}
		if(tx < tx_count_)
		{
			log = boost::str(boost::format("Invalid nonce. Expected >= %1%, got %2%") % tx_count_ % tx);
			return result_type::BadNonce;
		}
	}
	return result_type::OK;	
}


/*
	Original tendermint implementation:

	func (app *CounterApplication) AppendTx(tx []byte) types.Result {
		if app.serial {
			tx8 := make([]byte, 8)
			copy(tx8[len(tx8)-len(tx):], tx)
			txValue := binary.BigEndian.Uint64(tx8)
			if txValue != uint64(app.txCount) {
				return types.Result{
					Code: types.CodeType_BadNonce,
					Data: nil,
					Log:  fmt.Sprintf("Invalid nonce. Expected %v, got %v", app.txCount, txValue),
				}
			}
		}
		app.txCount += 1
		return types.OK
	}
*/

application_type::result_type application_type::append_tx(std::string const& in, std::string& out, std::string& log)
{
	std::unique_lock<std::mutex> lock(mutex_);
	if(serial_)
	{
		std::uint64_t tx(0);
		for(auto const& c: in)
		{
			tx <<= 8;
			tx += static_cast<std::uint8_t>(c);
		}
		if(tx != tx_count_)
		{
			log = boost::str(boost::format("Invalid nonce. Expected >= %1%, got %2%") % tx_count_ % tx);
			return result_type::BadNonce;
		}
	}
	++ tx_count_;
	return result_type::OK;	
}


/*
	Original tendermint implementation:	

	func (app *CounterApplication) Commit() types.Result {
		app.hashCount += 1

		if app.txCount == 0 {
			return types.OK
		} else {
			hash := make([]byte, 8)
			binary.BigEndian.PutUint64(hash, uint64(app.txCount))
			return types.NewResultOK(hash, "")
		}
	}

*/

application_type::result_type application_type::commit(std::string& out, std::string& log)
{
	std::unique_lock<std::mutex> lock(mutex_);
	++ hash_count_;
	if(tx_count_ > 0)
	{
		boost::endian::big_int64_buf_t buffer(tx_count_);
		out.assign(buffer.data(), sizeof(buffer));
	}
	return result_type::OK;	
}


}	// namespace counter