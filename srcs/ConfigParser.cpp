//
// Created by Anton on 19/08/2022.
//

#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string &path) : _line_number(1), _path(path) {}

//getter
const std::vector<Server *> &ConfigParser::getParsedServers() {
	if (_servers.empty())
		parse_config(_path.data());
	return _servers;
}

std::vector<Server *> &ConfigParser::getServers() {
	return _servers;
}

std::map<std::string, std::string> *ConfigParser::getMime() const {
	std::vector<std::string>	*my_vector;
	std::map<std::string, std::string> mimes;
	std::string key;
	std::string value;
	size_t pos = 0;

	my_vector= ft_split(ft_read_file(MIME_FILE), '\n');
	for(std::vector<std::string>::iterator it = my_vector->begin(); it < my_vector->end(); ++it)
	{
		pos = (*it).find(":");
		if (pos == std::string::npos) {
			delete my_vector;
			throw std::runtime_error("mime file is incorrect");
		}
		key = (*it).substr(0,pos);
		pos = (*it).find(" ");
		if (pos == std::string::npos) {
			delete my_vector;
			throw std::runtime_error("mime file is incorrect");
		}
		value = (*it).substr(pos + 1);
		mimes.insert(std::make_pair(key, value));
	}
	delete my_vector;
	if (mimes.empty())
		throw std::runtime_error("mime file is incorrect");
	return new std::map<std::string, std::string>(mimes);
}

/******************************************************************************************************************
 ************************************************** PARSING *******************************************************
 *****************************************************************************************************************/

void ConfigParser::parse_config(const char *path) {
	std::string file_content;
	str_iter it;
	
	//trying to open file and checks minimal requirements
	try {
		file_content = ft_read_file(path).append(" ");
	} catch (std::exception& e) {
		throw ConfigCriticalError("Cannot open config file");
	}
	//will read all file_content and delete already wrote part from file_content
	for (it = file_content.begin(); file_content.length() && it != file_content.end();) {
		//trying to find server block
		it = skip_comments_and_spaces(file_content, it);
		if (file_content.find("server") != 0)
			throw ConfigUnexpectedToken(find_unexpected_token(
					file_content, "server").data());
		it = file_content.erase(it, it + 6);
		it = skip_comments_and_spaces(file_content, it);
		// if server keyword is found, trying to find { and call parsing block function
		if (*it != '{')
			throw ConfigUnexpectedToken(find_unexpected_token(file_content,"{").data());
		parse_server_block(file_content, ++it);
		it = skip_comments_and_spaces(file_content, it);
	}
}

ConfigParser::str_iter ConfigParser::skip_comments_and_spaces(std::string& file_content, str_iter it) {
	while ((isspace(*it) && it != file_content.end()) || *it == '#') {
		if (*it == '#')
			while (it != file_content.end() && *it != '\n')
				it++;
		if (*it == '\n')
			_line_number++;
		it++;
	}
	return file_content.begin() == it ? it : file_content.erase(file_content.begin(), it);
}

std::string& ConfigParser::find_unexpected_token(std::string s, const char* token) {
	std::stringstream sstm; //FIXME maybe only this?
	sstm << _line_number;
	_error_msg = "Error, unexpected token. ";
	_error_msg.append(token).append(" expected at line ");
	_error_msg.append(sstm.str().append(":\n"));
	_error_msg.append(s.find('\n') == std::string::npos ? s :
					  s.substr(0, s.find('\n')));
	return _error_msg;
}

/******************************************************************************************************************
 **************************************** SERVER BLOCK PARSING ****************************************************
 *****************************************************************************************************************/

std::vector<std::string> ConfigParser::parse_parameter_args(std::string& file_content, str_iter &it) {
	std::vector<std::string> args;
	while (it != file_content.end() && *it != '#') {
		if (isspace(*it)) {
			args.push_back(file_content.substr(0, it - file_content.begin()));
			while (isspace(*it) && *it != '\n')
				it++;
			it = file_content.erase(file_content.begin(), it);
			if (*it == '\n')
				break;
			continue ;
		}
		it++;
	}
	it = file_content.erase(file_content.begin(), it);
	return (args);
}

void ConfigParser::parse_server_block(std::string &file_content, str_iter &it) {
	//creates new server and saves it
	Server* s = new Server;
	Location tmp_loc;
	std::vector<std::string> args;
	std::vector<std::pair<int, std::string> > locations_blocks;
	_servers.push_back(s);
	it = skip_comments_and_spaces(file_content, it);
	while (*it != '}' && it != file_content.end()) {
		//trying to find a keyword defined in Server::_server_keywords str array
		int keyword_index = find_server_keyword(file_content, it);
		//parsing of arguments of keyword
		if (keyword_index != LOCATION)
			args = parse_parameter_args(file_content, it);
		//filling of server settings
		switch (keyword_index) {
			case ::LISTEN: parse_listen_args(s, args); break;
			case ::PORT: parse_port_args(s, args); break;
			case ::SERVER_NAME: parse_servername_args(s, args); break;
			case ::ERR_PAGE: parse_errorpages_args(tmp_loc, args); break;
			case ::CLIENT_BODY_SIZE: parse_bodysize_args(tmp_loc, args); break;
			case ::FILE_UPLOAD: parse_fileupload_args(tmp_loc, args); break;
			case ::METHODS: parse_methods_args(tmp_loc, args); break;
			case ::INDEX: parse_index_args(tmp_loc, args); break;
			case ::AUTOINDEX: parse_autoindex_args(tmp_loc, args); break;
			case ::ROOT: parse_root_args(tmp_loc, args); break;
			case ::LOCATION: locations_blocks.push_back(skip_location_block(file_content, it)); break;
			case ::CGI_PATH: parse_cgi_path(s, args); break;
			default: {
				std::string tmp = "Expected one of\n [";
				for (int x = 0; x < MAX_KEYWORDS; x++) {
					tmp.append(Server::_server_keywords[x]);
					x < MAX_KEYWORDS - 1 ? tmp.append(", ") : tmp.append("] \nin server block");
				}
				throw ConfigUnexpectedToken(find_unexpected_token(file_content,tmp.data()).data());
			}
		}
		it = skip_comments_and_spaces(file_content, it);
		if (it == file_content.end())
			throw ConfigUnexpectedToken(find_unexpected_token(
					file_content, "}").data());
	}
	it++;
	if (s->getHost().empty() || s->getPorts().empty() || tmp_loc.getRoot().empty())
		throw ConfigNoRequiredKeywords();
	s->setDefault(tmp_loc);
	parse_locations(s, tmp_loc, locations_blocks);
}

std::pair<int, std::string> ConfigParser::skip_location_block(std::string &file_content, ConfigParser::str_iter &it) {
	std::pair<int, std::string> res = std::make_pair(_line_number, file_content);
	if (*it == '{')
		throw ConfigUnexpectedToken(find_unexpected_token(file_content,"location path").data());
	while (it != file_content.end() && !isspace(*it)) it++;
	while (isspace(*it) && *it != '\n') it++;
	if (*it++ != '{')
		throw ConfigUnexpectedToken(find_unexpected_token(file_content,"{").data());
	while (it != file_content.end() && *it != '{' && *it != '}') {
		if (*it == '\n') {
			_line_number++;
			it = file_content.erase(file_content.begin(), ++it);
			continue;
		}
		it++;
	}
	if (*it == '{')
		throw ConfigUnexpectedToken(find_unexpected_token(file_content,"You can't have "
										   "another blocks inside location block. }").data());
	if (it == file_content.end())
		throw ConfigUnexpectedToken(find_unexpected_token(file_content,"}").data());
	it = file_content.erase(file_content.begin(), ++it);
	return res;
}

void ConfigParser::parse_locations(Server *s, const Location &def, std::vector<std::pair<int, std::string> > &loc) {
	for (std::vector<std::pair<int, std::string> >::iterator location = loc.begin(); location != loc.end(); location++) {
		_line_number = (*location).first;
		std::string &block = (*location).second;
		Location result = def;
		str_iter it = block.begin();
		while (!isspace(*it)) it++;
		result.setLocation(block.substr(0, it - block.begin()));
		it = skip_comments_and_spaces(block, it);
		it = skip_comments_and_spaces(block, ++it);
		if (*it == '}')
			throw ConfigUnexpectedToken(find_unexpected_token(block,"Empty location block "
																	"detected. Parameters").data());
		while (it != block.end() && *it != '}') {
			int keyword_index = find_server_keyword(block, it);
			std::vector<std::string> args = parse_parameter_args(block, it);
			switch (keyword_index) {
				case ROOT: parse_root_args(result, args); break;
				case AUTOINDEX: parse_autoindex_args(result, args); break;
				case INDEX: parse_index_args(result, args); break;
				case FILE_UPLOAD: parse_fileupload_args(result, args); break;
				case CLIENT_BODY_SIZE: parse_bodysize_args(result, args); break;
				case ERR_PAGE: parse_errorpages_args(result, args); break;
				case METHODS: parse_methods_args(result, args); break;
				default: {
					std::string tmp = "Expected one of \n[";
					for (int x = 0; x < MAX_LOC_KEYWORDS; x++) {
						tmp.append(Location::_location_keywords[x]);
						x < MAX_LOC_KEYWORDS - 1 ? tmp.append(", ") : tmp.append("]\n in block");
					}
					throw ConfigUnexpectedToken(find_unexpected_token(Server::_server_keywords[keyword_index],
																	  tmp.data()).data());
				}
			}
			it = skip_comments_and_spaces(block, it);
			if (it == block.end())
					throw ConfigUnexpectedToken(find_unexpected_token(block, "}").data());
		}
		s->setLocations(result.getLocation(), result);
	}
}


/******************************************************************************************************************
 ************************************* PARAM ARGUMENTS PARSING ****************************************************
 *****************************************************************************************************************/

//trying to find a keyword and check's if they are arguments after it
int ConfigParser::find_server_keyword(std::string &file_content, str_iter &it) {
	unsigned long pos = file_content.find_first_of(" \t\r\v\f\n");
	std::string tmp = file_content.substr(0, pos);
	it += static_cast<std::string::difference_type>(pos);
	while (*it && *it != '\n' && isspace(*it))
		it++;
	if (pos == std::string::npos || *it == '\n')
		throw ConfigUnexpectedToken(find_unexpected_token(file_content,
														  "argument after keyword").data());
	int y;
	for (y = 0; y < MAX_KEYWORDS; y++)
		if (tmp == Server::_server_keywords[y])
			break;
	if (y == MAX_KEYWORDS) {
		tmp = "Expected one of\n [";
		for (int x = 0; x < MAX_KEYWORDS; x++) {
			tmp.append(Server::_server_keywords[x]);
			x < MAX_KEYWORDS - 1 ? tmp.append(", ") : tmp.append("]\n in server block");
		}
		throw ConfigUnexpectedToken(find_unexpected_token(
				file_content,tmp.data()).data());
	}
	it = skip_comments_and_spaces(file_content, it);
	return y;
}

void ConfigParser::parse_listen_args(Server *s, std::vector<std::string> &args) {
	if (args.size() != 1)
		throw ConfigUnexpectedToken(find_unexpected_token(ft_strjoin(args.begin(),
										 args.end(), " "), "only one ip address/host").data());
	if (*args.begin() != "localhost")
		s->setHost(*args.begin());
	else
		s->setHost("127.0.0.1");
}

void ConfigParser::parse_port_args(Server *s, std::vector<std::string> &args) {
	for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); it++) {
		int port = atoi((*it).data());
		if ((*it).find_first_not_of("0123456789") != std::string::npos ||
		((port < 1024 || port > 65535) && port != 80))
			throw ConfigUnexpectedToken(find_unexpected_token(ft_strjoin(args.begin(), args.end(),
					" "), "decimal value of port from 1024 to 65535 or 80").data());
		s->setPorts(port);
	}
}

void
ConfigParser::parse_servername_args(Server *s, std::vector<std::string> &args) {
	for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); it++) {
		for (std::vector<Server *>::iterator it_serv = _servers.begin(); it_serv != _servers.end(); it_serv++) {
			for (std::vector<std::string>::const_iterator it_sname = (*it_serv)->getServerName().begin();
				 it_sname != (*it_serv)->getServerName().end(); it_sname++) {
				if (*it == *it_sname && (*it).empty())
					throw ConfigUnexpectedToken(find_unexpected_token(*it,
									"Can't have 2 servers without server_name").data());
				else if (*it == *it_sname)
					throw ConfigUnexpectedToken(find_unexpected_token(
							*it,"Server is already exist."
										 " Uniq server_name").data());
//FIXME test empty servername
			}
		}
		s->setServerName(*it);
	}
	
}

void
ConfigParser::parse_errorpages_args(Location& loc, std::vector<std::string> &args) {
	int err_code;
	std::string err_page;

	if (args.size() == 2) {
		err_code = atoi((*args.begin()).data());
		if (args.begin()->find_first_not_of("0123456789") != std::string::npos ||
			((err_code < 400 || err_code > 417) && (err_code < 500 || err_code > 505)))
			throw ConfigUnexpectedToken(find_unexpected_token(
					*args.begin(), "error code from 400 to 417 or from 500 to 505").data());
		try {
			err_page = ft_read_file(*(args.begin() + 1));
		} catch (std::exception& e) {
			_error_msg = "Cannot open error page ";
			_error_msg.append(*(args.begin() + 1));
			throw ConfigCriticalError(_error_msg.data());
		}
	} else
		throw ConfigUnexpectedToken(find_unexpected_token(ft_strjoin(args.begin(), args.end(),
																	 " "),
														  "error_page [STATUS CODE] [PATH] pattern").data());
	loc.setErrorPages(static_cast<short>(err_code), err_page);
}

void ConfigParser::parse_bodysize_args(Location& loc, std::vector<std::string> &args) {
	long body_size = atol((*args.begin()).data());
	if (args.size() != 1 || (*args.begin()).find_first_not_of("0123456789") != std::string::npos || body_size < 0)
		throw ConfigUnexpectedToken(find_unexpected_token(ft_strjoin(args.begin(), args.end(), " "), "positive number").data());
	loc.setMaxBodySize(static_cast<unsigned long>(body_size));
}

void
ConfigParser::parse_fileupload_args(Location& loc, std::vector<std::string> &args) {
	std::string str = *args.begin();
	if (args.size() != 1 || (str != "on" && str != "off"))
		throw ConfigUnexpectedToken(find_unexpected_token(
				ft_strjoin(args.begin(), args.end(), " "), "on or off").data());
	loc.setFileUpload(str == "on");
}

void ConfigParser::parse_methods_args(Location &loc, std::vector<std::string> &args) {
	for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); it++) {
		if (*it == "GET")
			loc.setAllowedMethods(GET);
		else if (*it == "POST")
			loc.setAllowedMethods(POST);
		else if (*it == "DELETE")
			loc.setAllowedMethods(DELETE);
		else
			throw ConfigUnexpectedToken(find_unexpected_token(*it, "GET/POST/DELETE methods").data());
	}
}

void ConfigParser::parse_index_args(Location &loc, std::vector<std::string> &args) {
	for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); it++)
		loc.setIndex(*it);
}

void ConfigParser::parse_autoindex_args(Location &loc, std::vector<std::string> &args) {
	std::string str = *args.begin();
	if (args.size() != 1 || (str != "on" && str != "off"))
		throw ConfigUnexpectedToken(find_unexpected_token(
				ft_strjoin(args.begin(), args.end(), " "), "on or off").data());
	loc.setAutoindex(str == "on");
}

void ConfigParser::parse_root_args(Location &loc, std::vector<std::string> &args) {
	if (args.size() != 1)
		throw ConfigUnexpectedToken(find_unexpected_token(ft_strjoin(args.begin(), args.end(),
														 " "), "only root directory path").data());
	std::string root = *args.begin();
	if (*(root.end() - 1) == '/')
		root.erase(root.end() - 1);
	loc.setRoot(root);
}

void ConfigParser::parse_cgi_path(Server *s, std::vector<std::string> &args) {
	std::stringstream ss;
	ss << "supported cgi types are ";
	for (int i = 0; i < MAX_CGI_TYPES; i++) {
		ss << Server::_cgi_types[i];
		if (i != MAX_CGI_TYPES - 1)
			ss << ", ";
	}
	if (args.size() != 2) {
		throw ConfigUnexpectedToken(find_unexpected_token((ft_strjoin(args.begin(), args.end(), " ")), 
				"cgi_path [TYPE] [ABSOLUTE PATH TO CGI BIN]").data());
	}
	int i;
	for (i = 0; i < MAX_CGI_TYPES && Server::_cgi_types[i] != *args.begin(); i++);
	if (i == MAX_CGI_TYPES)
		throw ConfigUnexpectedToken(find_unexpected_token((ft_strjoin(args.begin(), args.end(), " ")),
														  (ss.str()).data()).data());
	if (access((args.begin() + 1)->data(), X_OK) == -1) {
		ss.str("");
		ss << "Can't execute " << _error_msg.append(*(args.begin() + 1));
		ss << _error_msg.append(", absolute path to bin");
		throw ConfigUnexpectedToken(find_unexpected_token((ft_strjoin(args.begin(), args.end(), " ")),
														  ss.str().data()).data());
	}
	s->setCgiPaths(*args.begin(), *(args.begin() + 1));
}

/******************************************************************************************************************
 *********************************************** EXCEPTIONS *******************************************************
 *****************************************************************************************************************/

ConfigParser::ConfigUnexpectedToken::ConfigUnexpectedToken(const char *m): msg(m) {}

const char *ConfigParser::ConfigUnexpectedToken::what() const throw() {
	return msg;
}

ConfigParser::ConfigCriticalError::ConfigCriticalError(const char *m): msg(m) {}

const char *ConfigParser::ConfigCriticalError::what() const throw() {
	return msg;
}

const char *ConfigParser::ConfigNoRequiredKeywords::what() const throw() {
	return ("Your config must have as least one server block "
			"with listen, port, root parameters like:\n"
			"server {\n\tlisten [IP ADDRESS]\n\tport [PORT]\n\troot usr/site/www\n}");
}

