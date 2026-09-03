/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorBuilder.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 20:27:28 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/13 20:29:48 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORBUILDER_HPP
# define ERRORBUILDER_HPP

# include <string>

class ErrorBuilder
{
private:
	ErrorBuilder();
	~ErrorBuilder();

	static std::string getStatusMessage(int code);
	static std::string getDefaultPage(int code, const std::string& message);

public:
	static std::string build(int errorCode, const std::string& customPagePath = "");

	// error_page 413 error_pages/413.html; guarda o valor cru do config.
	// Se não começar com "/" ou "./", é relativo ao root da location/server
	// (ex.: root "./www" + "error_pages/413.html" -> "./www/error_pages/413.html").
	// Usado por qualquer chamador que precise resolver esse caminho antes de
	// passar pra build() -- centraliza a lógica que antes estava duplicada.
	static std::string resolvePagePath(const std::string& root, const std::string& pagePath);
};


#endif