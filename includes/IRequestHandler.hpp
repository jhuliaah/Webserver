// Essa classe vai funcionar cmo a interface para lidar com requests.

#ifndef IREQUESTHANDLER_HPP
#define IREQUESTHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationConfig.hpp"

class Client;

class IRequestHandler
{
	public:
		virtual ~IRequestHandler() {} /*	precisa de virtual destructor para
											chamar o destrutor certo. */

		/*	é virtual para que cada handler faça overload com a implementação
			específica. E tem que ser pure virtual (=0) para que a classe
			IRequestHandler vire interface, não dá para instanciar ela. */




			virtual bool handle(const HttpRequest& req,
							const LocationConfig& loc,
							Client& client) = 0;
							/*	por que usar client e não response, como eu
							tinha pensando antes? porque o client é quem vai
							receber	a response, e cada handler escreve a response
							de um jeito, então é melhor passar client por ref
							(sem const) para o handler escrever a resposta
							direto nele. */
};

#endif



