/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequest.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduribei <eduribei@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 17:45:50 by eduribei          #+#    #+#             */
/*   Updated: 2026/06/13 18:20:17 by eduribei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_REQUEST_HPP
# define CGI_REQUEST_HPP

# include <string>

class CgiRequest
{
	private:
		std::string _method;


	public:
		CgiRequest();
		~CgiRequest();

		const std::string&  getMethod() const;

};

#endif