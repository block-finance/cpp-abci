#include "counter_application.hpp"
#include <lib-tmsp/server.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/trivial.hpp>


int main(int argc, char const* argv[])
{
	if(argc < 3)
	{
		BOOST_LOG_TRIVIAL(info) << "Provide TMSP listening endpoint on command line. Example: 0.0.0.0 46658";
		return 1;
	}

	try
	{
		boost::asio::io_service io_service;
		tmsp::server_type const server(
			io_service, 
			boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(argv[1]), std::atoi(argv[2])),
			std::make_shared<counter::application_type>()
			);
		
		// In this simple application, all work is scheduled on the main thread using boost asio (just below).
		// Since the application is thus protected from being re-entered, there is no need for a mutex to synchronize access. 
		// It was left there as a kind reminder that synchronization must be used if the thread count ever increases.
		io_service.run();
	}

	catch(...)
	{
		BOOST_LOG_TRIVIAL(error) << boost::current_exception_diagnostic_information();
		return 2;
	}

	return 0;
}