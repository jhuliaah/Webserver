/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduribei <eduribei@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 17:45:48 by eduribei          #+#    #+#             */
/*   Updated: 2026/06/13 18:13:26 by eduribei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/HttpRequest.hpp"
#include "../include/HttpResponse.hpp"


int main(void)
{
	HttpRequest req;

	req.method = "GET";
	req.uri = "/cgi-bin/hello.py?name=eduardo";
	req.path = "/cgi-bin/hello.py";
	req.query_string = "name=eduardo";

	RouteConfig cgi_route;
	cgi_route.path_prefix = "/cgi-bin/";
	cgi_route.root = "./www";
	cgi_route.cgi_extension = ".py";
	cgi_route.cgi_path = "/usr/bin/python3";
	cgi_route.is_cgi = true;

	Router router;

	router.add_route(cgi_route);

	RouteConfig matched = router.match(req);






}


