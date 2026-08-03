/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 17:45:50 by eduribei          #+#    #+#             */
/*   Updated: 2026/07/28 16:58:28 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

# include <string>
# include "IRequestHandler.hpp"

class CgiHandler : public IRequestHandler
{
	private:
		std::string _method;


	public:
		CgiHandler();
		~CgiHandler();

		const std::string& getMethod() const;

		char	**CgiEnvBuilder(const HttpRequest& req);
		void	parseCgiOutput(	const std::string& buffer, HttpResponse& response);
		bool	handle(const HttpRequest& req, const LocationConfig& loc, Client& client);

};

#endif