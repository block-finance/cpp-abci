#pragma once


#include <lib-tmsp/application.hpp>
#include <shared_mutex>


namespace counter
{


class application_type : public tmsp::application_type
{
public:
	application_type();

	virtual void info(std::string& out);
	virtual void set_option(std::string const& key, std::string const& value, std::string& log);
	virtual result_type query(std::string const& in, std::string& out, std::string& log);
	
	virtual void begin_block(std::uint64_t const& height);
	virtual result_type end_block(std::uint64_t const& height);
	
	virtual result_type check_tx(std::string const& in, std::string& out, std::string& log);
	virtual result_type append_tx(std::string const& in, std::string& out, std::string& log);
	virtual result_type commit(std::string& out, std::string& log);

private:
	std::mutex mutex_;
	std::uint64_t hash_count_;
	std::uint64_t tx_count_;
	bool serial_;
};


}	// namespace counter