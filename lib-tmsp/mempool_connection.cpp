#include "mempool_connection.hpp"
#include <boost/log/trivial.hpp>


namespace tmsp
{


mempool_connection_type::mempool_connection_type(boost::asio::ip::tcp::socket& socket) : connection_type(socket)
{
	BOOST_LOG_TRIVIAL(info) << socket_.remote_endpoint() << ": Starting TMSP mempool connection";
};


bool mempool_connection_type::prepare_response_(types::Request const& request, types::Response& response)
{
	if(request.type() == types::Flush)
	{
		response.set_type(types::Flush);
		response.set_code(types::OK);
		return true;
	}
	else if(request.type() == types::CheckTx)
	{
		response.set_type(types::CheckTx);
		response.set_code(types::OK);
		response.set_log("CheckTx on mempool connection");
		return true;
	}
	else if(request.type() == types::Commit)
	{
		response.set_type(types::Commit);
		response.set_code(types::OK);
		return true;
	}

	BOOST_THROW_EXCEPTION(std::logic_error("unsupported mempool request: " + request.DebugString()));
}


}	// namespace tmsp