/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*  StaticHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduribei <eduribei@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 17:45:48 by eduribei          #+#    #+#             */
/*   Updated: 2026/06/13 18:13:26 by eduribei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/StaticHandler.hpp"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>

StaticHandler::StaticHandler() {}

StaticHandler::~StaticHandler() {}

bool StaticHandler::handle(const HttpRequest& req, const LocationConfig& loc, Client& client){
	(void)loc;
	(void)client;

	//temporario ate ter o config
	std::string filePath = "./www" + req.getUri();
	//se pedirem a raiz "/", mandamos o index.html por defeito
	if (req.getUri() == "/") {
		filePath = "./www/index.html";
	}
	return false;
}


