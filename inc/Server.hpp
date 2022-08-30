//
// Created by amyroshn on 8/16/22.
//

#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP
#include "webserv.hpp"
#include "Location.hpp"

class Server {
private:
	std::set <int>						_ports;
	std::string							_host;
	std::vector<std::string>			_server_name;
	std::map<std::string, Location>		_locations;
	Location							_default;
public:
	Server();
	static const std::string    _server_keywords[];
	static const std::string    _all_keywords[]; //FIXME unused

/******************************************************************************************************************
 ************************************************** GETTERS *******************************************************
 *****************************************************************************************************************/

	const std::string &getHost() const;
	const std::set<int> &getPorts() const;
	const std::vector<std::string> &getServerName() const;
	const std::map<std::string, Location> &getLocations() const;
	const Location &getConstDefault() const;

/******************************************************************************************************************
 ************************************************** SETTERS *******************************************************
 *****************************************************************************************************************/
 
	void setLocations(const std::string& path, const Location &location);
	void setHost(const std::string &ip);
	void setPorts(const int& port);
	void setServerName(const std::string &serverName);
	void setDefault(const Location &location);

	};


#endif //WEBSERV_SERVER_HPP
