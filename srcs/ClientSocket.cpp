//
// Created by Anton on 29/08/2022.
//

#include "ClientSocket.hpp"


ClientSocket::ClientSocket(const ListeningSocket *parentSocket, int socket_fd,
						   const sockaddr_in &addr, socklen_t addrLen) : ASocket(socket_fd),
																	_parent_socket(parentSocket),
																	_close_immediately(false), _addr(addr),
																	_addr_len(addrLen) {
	std::stringstream ss;
	ss << "Client " << _socket_fd << " has been connected to ";
	ss << _parent_socket->getHost() << ":" << _parent_socket->getPort();
	Logger::println(Logger::TXT_BLACK, Logger::BG_WHITE, ss.str());
}

/* FIXME POTENTIAL BUg IF MSG SIZE IS GREATER THAN 65535 BITES */
bool ClientSocket::check_headers(const Request &req, const std::map<std::string, std::string> *mime) {
	std::map<std::string, std::string>::const_iterator it = req._header.find("Connection");
	if (it != req._header.end() && it->second == "close")
		_close_immediately = true;
	it = req._header.find("Content-Length");
	if (it != req._header.end()) {
		size_t content_length = std::stoul(it->second);
		if (req._request_body.length() < content_length) {
			Logger::print(Logger::TXT_BLACK, Logger::BG_YELLOW, "Content-Length is ", content_length,
							" got ");
			Logger::println(Logger::TXT_BLACK, Logger::BG_YELLOW, req._request_body.length(), " bytes");
			return false;
		}
		if (req._request_body.length() > content_length) {
			_request.erase(content_length - 1);
			Request new_req(_request.data(), this, mime);
			_response = new_req._rep;
		}
	}
	return true;
}



bool ClientSocket::recv_msg(const std::map<std::string, std::string> *mime){
	char *data[BUF_SIZE];
	size_t not_space_pos;

	ssize_t read_status = read(_socket_fd, data, BUF_SIZE);
	if (read_status <= 0) //Can't read or got empty msg
		throw CannotAccessDataException("Connection is closed");
	_request.append(reinterpret_cast<const char *>(data), read_status);
	not_space_pos = _request.find_first_not_of(" \f\n\r\t\v");
	if (not_space_pos != std::string::npos &&
		(_request.find("\r\n\r\n") != std::string::npos || _request.find("\n\n") != std::string::npos)) {
		Request req(_request.data(), this, mime);
		_response = req._rep;
		if (check_headers(req, mime)) {
			Logger::print("\nGot request from client ", _socket_fd, ":\t");
			Logger::println(Logger::TXT_BLACK, Logger::BG_CYAN, _request);
			_request.clear();
			return true;
		}
	} else if (not_space_pos == std::string::npos)
		_request.clear();
	return false; //got only newline characters
}

bool ClientSocket::answer() {
	ssize_t write_status = write(_socket_fd, _response.data(), _response.size());
	if (write_status == -1)
		throw CannotAccessDataException("Can't write data in socket");
	if (_response.length() == 0)
		throw std::runtime_error("Empty response, closing connection");
	if (write_status == static_cast<ssize_t>(_response.size()) && _close_immediately)
		throw std::runtime_error("Connection was closed by http \"Connection: close\" header");
	if (write_status == static_cast<ssize_t>(_response.size())) {
		Logger::print("\nMessage to", _socket_fd, ":\t");
		Logger::println(Logger::TXT_BLACK, Logger::BG_CYAN,_response.substr(0, _response.find('\n')));
		Logger::println("");
		_response.clear();
		return true;
	}
	_response.erase(0, static_cast<size_t>(write_status));
	Logger::println(Logger::TXT_BLACK, Logger::BG_YELLOW, "Not all bites were sent to", _socket_fd,
					"Message is erased and will sent another part again");
	return false;
}

void ClientSocket::close() {
	if (_socket_fd != -1) {
		Logger::print(Logger::TXT_BLACK, Logger::BG_WHITE, "Client", _socket_fd, "has been disconnected from ");
		Logger::println(Logger::TXT_BLACK, Logger::BG_WHITE, 
						_parent_socket->getHost() ,_parent_socket->getPort());
		::close(_socket_fd);
		_socket_fd = -1;
	}
}

ClientSocket::~ClientSocket() {
	close();
}

std::string ClientSocket::getClientAddr() const {
	(void) _addr_len;
	return std::string(inet_ntoa(_addr.sin_addr));
}

std::string ClientSocket::getClientPort() const {
	std::stringstream ss;
	ss << ntohs(_addr.sin_port);
	return std::string(ss.str());
}

std::string ClientSocket::getPort() const {
	std::stringstream ss;
	ss << _parent_socket->getPort();
	return std::string(ss.str());
}

const std::vector<const Server *> &ClientSocket::getServers() const {
	return _parent_socket->getServers();
}

std::string ClientSocket::getAddr() const {
	return _parent_socket->getAddr();
}
