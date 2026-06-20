/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduribei <eduribei@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 17:34:33 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/20 19:27:48 by eduribei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "WebServer.hpp"

class Socket
{
	protected:
		int	_serverFd;

	public:
		Socket();
		void	SocketConfig();
		void	nonBlocking();
		int		getFd() const;
};
