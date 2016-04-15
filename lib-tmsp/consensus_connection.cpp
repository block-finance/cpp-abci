#include "consensus_connection.hpp"
#include <boost/log/trivial.hpp>


namespace tmsp
{


consensus_connection_type::consensus_connection_type(boost::asio::ip::tcp::socket& socket) : connection_type(socket)
{
	BOOST_LOG_TRIVIAL(info) << socket_.remote_endpoint() << ": Starting TMSP consensus connection";
};


bool consensus_connection_type::prepare_response_(types::Request const& request, types::Response& response)
{
	BOOST_LOG_TRIVIAL(debug) << socket_.remote_endpoint() << ": " << request.DebugString();

	if(request.type() == types::Echo)
	{
		BOOST_LOG_TRIVIAL(debug) << socket_.remote_endpoint() << ": Echo \"" << request.data() << "\"";
		response.set_type(types::Echo);
		response.set_code(types::OK);
		response.set_data(request.data());
		return true;
	}
	else if(request.type() == types::Info)
	{
		response.set_type(types::Info);
		response.set_code(types::OK);
		return true;
	}
	else if(request.type() == types::Flush)
	{
		response.set_type(types::Flush);
		response.set_code(types::OK);
		return true;
	}
	else if(request.type() == types::CheckTx)
	{
		response.set_type(types::CheckTx);
		response.set_code(types::OK);
		response.set_log("CheckTx on consensus connection");
		return true;
	}
	else if(request.type() == types::AppendTx)
	{
		response.set_type(types::AppendTx);
		response.set_code(types::OK);
		response.set_log("AppendTx on consensus connection");
		return true;
	}
	else if(request.type() == types::Commit)
	{
		response.set_type(types::Commit);
		response.set_code(types::OK);
		return true;
	}
	else if(request.type() == types::BeginBlock)
	{
		BOOST_LOG_TRIVIAL(debug) << socket_.remote_endpoint() << ": BeginBlock @ " << request.height();
		return false;
	}
	else if(request.type() == types::EndBlock)
	{
		BOOST_LOG_TRIVIAL(debug) << socket_.remote_endpoint() << ": EndBlock @ " << request.height();
		response.set_type(types::EndBlock);
		response.set_code(types::OK);
		return true;
	}

	BOOST_THROW_EXCEPTION(std::logic_error("unsupported TMSP request: " + request.DebugString()));
}


}	// namespace tmsp