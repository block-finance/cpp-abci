#pragma once


#include "connection.hpp"


namespace tmsp
{


class server_type
{
public:
	server_type(boost::asio::io_service& io_service, boost::asio::ip::tcp::endpoint const& endpoint, application_ptr_type const& application);

private:
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket socket_;
	application_ptr_type const application_;

	void accept_connection_();
};


}	// namespace tmsp

