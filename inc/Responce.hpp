#ifndef RESPONCE_HPP
#define RESPONCE_HPP

#include "webserv.hpp"

struct responce
{
	std::string		_http_version;
	// std::map<int, std::string>	_status;
	int				_status_code;
	std::string		_status_message;
	std::string		_date;
	std::string		_server;
	std::string		_content_type;
	size_t			_content_length;
};

#endif //WEBSERV_WEBSERV_HPP