/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduribei <eduribei@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 17:45:48 by eduribei          #+#    #+#             */
/*   Updated: 2026/06/13 18:13:26 by eduribei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/CgiHandler.hpp"


CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

const std::string& CgiHandler::getMethod() const
{
    /* TODO */
    return _method; /* TODO */
}

char **CgiHandler::CgiEnvBuilder()
{
    /* TODO */
    return NULL; /* TODO */
}

void CgiHandler::parseCgiOutput(const std::string& buffer, HttpResponse& response)
{
    (void)buffer;
    (void)response;
    
    /* TODO */
    /*  essa funçao vai tratar a saida do CGI e preencher o objeto HttpResponse 
        que vive dentro do objeto Client, que o giHandler::handle() recebeu. */
}

bool CgiHandler::handle(const HttpRequest& req, const LocationConfig& loc, Client& client)
{
    /* TODO */
    (void)req;
    (void)loc;
    (void)client;

    return false;     /* Já sei que vai ser false... */
}
