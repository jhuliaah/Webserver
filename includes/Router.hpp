#ifndef ROUTER_HPP
# define ROUTER_HPP

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

/*	estou em dúvida se esse é o melhor lugar para deixar o esse enum,
	mas acho que é aqui mesmo. o Server faz include de Router.hpp, e são
	só essas duas classes que vão usar o RouteType. */
enum RouteType
{
	STATIC,
	CGI,
	DIR,
	ERROR
};

class Router
{
	private:
		Router();
		~Router();
		/*deixando os construtores privados, já que combinamos que ela será
		uma classe stateless. bom, tecnicamente falando, o fato de poder ser
		instanciada não torna a classe necssariamente stateful, o que define
		uma classe como stateless é não guardar estados (como atributos).
		mas se ela não guarda nenhum estado, não tem por que instanciá-la.
		ela pode ser chamada apenas como:
		
		Router::handle();

		em vez de:

		Router router;		<-----instancia objeto na memória
		router.handle(); 
		
		é importante olhar aí embaixo que tem um "static" antes de cada método,
		sem isso não dá para chamar o métido sem instaciar a classe.

		*/
	
	public:
		static LocationConfig matchLoc(ServerConfig server, std::string uri);
		// qual é a location dessa URI?
	
		static RouteType classify(LocationConfig loc, std::string path, std::string method);
		// é CGI, static, diretório ou error?

};

#endif
