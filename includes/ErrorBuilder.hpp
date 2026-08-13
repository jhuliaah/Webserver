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
};


#endif