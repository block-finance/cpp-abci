#include "server.hpp"
#include "consensus_connection.hpp"
#include "mempool_connection.hpp"


namespace tmsp
{


server_type::server_type(boost::asio::io_service& io_service, boost::asio::ip::tcp::endpoint const& endpoint) :
	acceptor_(io_service, endpoint), socket_(io_service)
{
	accept_mempool_connection_();
}


void server_type::accept_consensus_connection_()
{
	acceptor_.async_accept(socket_, 
		[this](boost::system::error_code const& error)
		{
			if(!error)
			{
				std::make_shared<consensus_connection_type>(std::move(socket_))->start();
			}
		});
}


void server_type::accept_mempool_connection_()
{
	acceptor_.async_accept(socket_, 
		[this](boost::system::error_code const& error)
		{
			if(!error)
			{
				std::make_shared<mempool_connection_type>(std::move(socket_))->start();
				accept_consensus_connection_();
			}
		});
}


}	// namespace tmsp
