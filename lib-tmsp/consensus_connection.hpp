#pragma once


#include "connection.hpp"


namespace tmsp
{


class consensus_connection_type : public connection_type
{
public:
	consensus_connection_type(boost::asio::ip::tcp::socket& socket);

protected:
	virtual bool prepare_response_(types::Request const& request, types::Response& response);
};


}	// namespace tmsp