//
// Created by amyroshn on 8/16/22.
//

#ifndef WEBSERV_WEBSERV_MACHINE_HPP
#define WEBSERV_WEBSERV_MACHINE_HPP
#include "webserv.hpp"
#include "Server.hpp"
#include "ConfigParser.hpp"
#include "Socket.hpp"

class Socket;
class Server;

class Webserv_machine {
private:
	std::map<int, Socket *>     _machine_sockets; // <socket_fd, socket>
	std::vector<Server *>	    _servers;
	std::string 				_error_msg;
	fd_set				    	_server_fd_set;

public:
	/*      construct / destruct        */
	Webserv_machine(const char *path);

	virtual ~Webserv_machine();

	void up();
	
private:
	void run_listening_sockets();
public:
	

/******************************************************************************************************************
 ************************************************** GETTERS *******************************************************
 *****************************************************************************************************************/

	const std::map<int, Socket *> &getMachineSockets() const;
	const std::vector<Server *> &getServers() const;
	const std::string &getErrorMsg() const;
	
/******************************************************************************************************************
 ************************************************** SETTERS *******************************************************
 *****************************************************************************************************************/

	void setMachineSockets(int port, Socket *socket);
	void setServers(Server *server);

};


#endif //WEBSERV_WEBSERV_MACHINE_HPP
