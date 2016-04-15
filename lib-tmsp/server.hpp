#pragma once


#include "connection.hpp"


namespace tmsp
{


class server_type
{
public:
	server_type(boost::asio::io_service& io_service, boost::asio::ip::tcp::endpoint const& endpoint);

private:
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket socket_;

	void accept_connection_();
};


}	// namespace tmsp

