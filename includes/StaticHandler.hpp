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

class StaticHandler : public IRequestHandler
{
	private:
		std::string _method;


	public:
		StaticHandler();
		~StaticHandler();

};

#endif