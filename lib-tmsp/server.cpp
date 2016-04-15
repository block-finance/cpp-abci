#include "server.hpp"
#include "connection.hpp"


namespace tmsp
{


server_type::server_type(boost::asio::io_service& io_service, boost::asio::ip::tcp::endpoint const& endpoint) :
	acceptor_(io_service, endpoint), socket_(io_service)
{
	accept_connection_();
}


void server_type::accept_connection_()
{
	acceptor_.async_accept(socket_, 
		[this](boost::system::error_code const& error)
		{
			if(!error)
			{
				std::make_shared<connection_type>(std::move(socket_))->start();
				accept_connection_();
			}
		});
}


}	// namespace tmsp
