/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 15:32:55 by ratanaka          #+#    #+#             */
/*   Updated: 2026/05/26 15:52:13 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>

int main(int argc, char* argv[]){
	if (argc != 2){
		std::cout << "EX{./webserv <config_file>}" << std::endl;
		return 1;
	}
	(void)argv;
}