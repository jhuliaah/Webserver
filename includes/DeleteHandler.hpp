/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 17:46:33 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/03 17:52:17 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DELETEHANDLER_HPP
# define DELETEHANDLER_HPP

# include "HttpRequest.hpp"
# include "Client.hpp"
# include "LocationConfig.hpp"
# include <string>
#include "UploadHandler.hpp"

class DeleteHandler {
	public:
		DeleteHandler();
		~DeleteHandler();
		
		bool handle(const HttpRequest& req, const LocationConfig& loc, Client& client);
};

#endif