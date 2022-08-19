//
// Created by amyroshn on 8/16/22.
//

#include "Server.hpp"
const std::string Server::_server_keywords[MAX_SERV_KEYWORDS] = {"listen", "port","server_name",
																 "error_page","client_max_body_size",
																 "file_upload","methods", "index", 
																 "autoindex"};

Server::Server() {
}

void Server::launch() {
	Socket *s = new Socket();
	s->setPort(*_ports.begin());
	s->setAddress(_address);
	_sockets.push_back(s);
	s->open();
}

/******************************************************************************************************************
 ************************************************** GETTERS *******************************************************
 *****************************************************************************************************************/


const std::vector<Socket *> &Server::getSockets() const {
	return _sockets;
}

const std::set<int> &Server::getPorts() const {
	return _ports;
}
const sockaddr_in &Server::getAddress() const {
	return _address;
}

const std::vector<std::string> &Server::getServerName() const {
	return _server_name;
}

Location &Server::getSettings() {
	return _settings;
}

const std::string&Server::getIp() const {
	return _ip;
}

const std::vector<Location> &Server::getLocations() const {
	return _locations;
}

/******************************************************************************************************************
 ************************************************** SETTERS *******************************************************
 *****************************************************************************************************************/


void Server::setIp(const std::string &ip) {
	_ip = ip;
}

void Server::setSockets(const std::vector<Socket *> &sockets) {
	_sockets = sockets;
}

void Server::setPorts(const int& port) {
	_ports.insert(port);
}

void Server::setAddress(const sockaddr_in &address) {
	_address = address;
}

void Server::setServerName(const std::string &serverName) {
	_server_name.push_back(serverName);
}

void Server::setSettings(const Location &settings) {
	_settings = settings;
}

void Server::setLocations(const Location &location) {
	_locations.push_back(location);
}
