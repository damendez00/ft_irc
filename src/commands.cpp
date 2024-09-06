#include "Server.hpp"
#include "Request.hpp"

std::string Server::_handlePass(const Request& req, int client_fd) {
    if (req.params.size() < 1) {
        std::vector<std::string> params;
        params.push_back(_clients[client_fd]->getNickName());
        params.push_back("Not enough parameters");
        return format_message(_name, ERR_NEEDMOREPARAMS, params);
    }
    if (_clients[client_fd]->getRegistered()) {
        std::vector<std::string> params;
        params.push_back(_clients[client_fd]->getNickName());
        params.push_back("Already registered");
        return format_message(_name, ERR_ALREADYREGISTRED, params);
    }
    if (req.params[0] != _password) {
        std::vector<std::string> params;
        params.push_back(_clients[client_fd]->getNickName());
        params.push_back("Password incorrect");
        return format_message(_name, ERR_PASSWDMISMATCH, params);
    }
    _clients[client_fd]->setAuth(true);
    print_debug("Client authenticated: " + itos(client_fd), colors::green, colors::bold);
    // Establecer el hostname y enviar mensaje de bienvenida con formato RPL_WELCOME
    std::vector<std::string> params;
    params.push_back(_clients[client_fd]->getNickname());
    params.push_back("Welcome to " + _name + " " + _clients[client_fd]->formatPrefix());
    return format_message(_name, RPL_WELCOME, params);
}

std::string Server::_handleNick(const Request& req, int client_fd) {
    // Verificar si se ha dado un nickname
    if (req.params.size() < 1) {
        std::vector<std::string> params;
        params.push_back(_clients[client_fd]->getNickName());
        params.push_back("No nickname given");
        return format_message(_name, ERR_NONICKNAMEGIVEN, params);
    }
    // Verificar si el nickname ya está en uso
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickName() == req.params[0] && it->first != client_fd) {
            std::vector<std::string> params;
            params.push_back(_clients[client_fd]->getNickName());
            params.push_back("Nickname is already in use");
            return format_message(_name, ERR_NICKNAMEINUSE, params);
        }
    }
    // Verificar si el nickname es válido
    if (req.params[0].length() > 9 || !isAlphaNumeric(req.params[0])) {
        std::vector<std::string> params;
        params.push_back(_clients[client_fd]->getNickName());
        params.push_back("Erroneous nickname");
        return format_message(_name, ERR_ERRONEUSNICKNAME, params);
    }
    //Guardar el format message del anterior nickname para notificar al cliente el cambio de nickname
    std::vector<std::string> params;
    params.push_back(req.params[0]);
    std::string acknoledgement = format_message(_clients[client_fd]->formatPrefix(), "NICK", params);
    // Establecer el nickname o cambiarlo
    _clients[client_fd]->setNickname(req.params[0]);
    if (_clients[client_fd]->isRegistered()) {
        return acknoledgement;
    }
    // Establecer el hostname y enviar mensaje de bienvenida con formato RPL_WELCOME
    params.clear();
    params.push_back(_clients[client_fd]->getNickname());
    params.push_back("Welcome to " + _name + " " + _clients[client_fd]->formatPrefix());
    return format_message(_name, RPL_WELCOME, params);
}

std::string Server::_handleUser(const Request& req, int client_fd) {
    // Verificar si el usuario ya está autenticado
    if (_clients[client_fd]->getRegistered()) {
        std::vector<std::string> params;
        params.push_back(_clients[client_fd]->getNickName());
        params.push_back("Already registered");
        return format_message(_name, ERR_ALREADYREGISTRED, params);
    }
    // Verificar si el usuario ha dado todos los parámetros
    if (req.params.size() < 4  || !isAlphaNumeric(req.params[0])) {
        std::vector<std::string> params;
        params.push_back(_clients[client_fd]->getNickName());
        params.push_back("Not enough parameters");
        return format_message(_name, ERR_NEEDMOREPARAMS, params);
    }
    // Establecer el username
    _clients[client_fd]->setUsername(req.params[0]); //RETRIBUIR AQUÍ TAMBIÉN EL MODE
    // Establecer el hostname y enviar mensaje de bienvenida con formato RPL_WELCOME
    std::vector<std::string> params;
    params.push_back(_clients[client_fd]->getNickname());
    params.push_back("Welcome to " + _name + " " + _clients[client_fd]->formatPrefix());
    return format_message(_name, RPL_WELCOME, params);
}

std::string Server::handleQuit(const Request& req, int client_fd) {
    //Enviar al usuario un reconocimiento de que se ha desconectado
    std::string server_response = format_message(_clients[client_fd]->formatPrefix(), "QUIT", req.params);
    send(client_fd, server_response.c_str(), server_response.length(), 0);
    // Desconectar al cliente
    handleDisconnection(client_fd);
    return "";
}

std::string	Server::_handleOperator(const Request& req, int client_fd) {
	if (!this->_clients[client_fd]->getRegistered())
		return (_printMessage("451", this->_clients[client_fd]->getNickName(), ":You have not registered"));
	if (req.params.size() < 2)
		return (_printMessage("461", this->_clients[client_fd]->getNickName(), "PASS :Not enough parameters"));
	if (req.params[0] != "ADMIN")
		return (_printMessage("464", this->_clients[client_fd]->getNickName(), ":Username/Password incorrect"));
	if (req.params[1] != "1234")
		return (_printMessage("464", this->_clients[client_fd]->getNickName(), ":Username/Password incorrect"));
	this->_clients[client_fd]->setIsOperator(true);
	return (_printMessage("381", this->_clients[client_fd]->getNickName(), ":You are now an IRC operator"));
}
