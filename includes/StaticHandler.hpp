/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduribei <eduribei@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 17:45:50 by eduribei          #+#    #+#             */
/*   Updated: 2026/06/13 18:20:17 by eduribei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATIC_HANDLER_HPP
# define STATIC_HANDLER_HPP

# include <string>
# include "IRequestHandler.hpp"
# include "HttpRequest.hpp"
# include "Client.hpp"

class StaticHandler {
	private:
		std::string getMimeType(const std::string& path);
		std::string buildAutoIndex(const std::string& path, const std::string& uri);

	public:
		StaticHandler();
		~StaticHandler();

		bool handle(const HttpRequest& req, const LocationConfig& loc, Client& client);
};

#endif