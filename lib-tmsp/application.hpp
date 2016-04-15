#pragma once


#include <cstdint>
#include <memory>
#include <string>
#include <tmsp/types.pb.h>


namespace tmsp
{


class application_type : public std::enable_shared_from_this<application_type>
{
public:
	application_type() {}

	typedef types::CodeType result_type;

	virtual void info(std::string& out) = 0;
	virtual void set_option(std::string const& key, std::string const& value, std::string& log) = 0;
	virtual result_type query(std::string const& in, std::string& out, std::string& log) = 0;
	
	virtual void begin_block(std::uint64_t const& height) = 0;
	virtual result_type end_block(std::uint64_t const& height) = 0;
	
	virtual result_type check_tx(std::string const& in, std::string& out, std::string& log) = 0;
	virtual result_type append_tx(std::string const& in, std::string& out, std::string& log) = 0;
	virtual result_type commit(std::string& out, std::string& log) = 0;
};


typedef std::shared_ptr<application_type> application_ptr_type;


}	// namespace tmsp
