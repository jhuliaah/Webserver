#include "../../includes/Config.hpp"
#include "../../includes/ServerConfig.hpp"
#include "../../includes/LocationConfig.hpp"

Config::Config()
{
	// CHAMA ARQUIVO DEFAULT
	// VALIDA (mesmo sendo o default, tem que validar,
	// porque o user pode ter alterado o .config default)
}

Config::Config(std::string)
{
	// CHAMA ARGV1
	// VALIDA
}

Config::~Config(){}


Config makeConfig(int argc, char* argv[])
{
	if (argc == 2)
		return Config(argv[1]);
	return Config();
}


static LocationConfig makeLocation(const std::string& path, bool autoindex)
{
	LocationConfig loc;

	loc._path = path;
	loc._autoindex = autoindex;
	return (loc);
}

/////// fim ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*                                                                             
                                 ▄▄                             ▄▄           
                     ▄▄          ██                            ██  ▀▀        
███▄███▄ ▄███▄ ▄████ ██ ▄█▀   ▄████ ▄█▀█▄   ▄████ ▄███▄ ████▄ ▀██▀ ██  ▄████ 
██ ██ ██ ██ ██ ██    ████     ██ ██ ██▄█▀   ██    ██ ██ ██ ██  ██  ██  ██ ██ 
██ ██ ██ ▀███▀ ▀████ ██ ▀█▄   ▀████ ▀█▄▄▄   ▀████ ▀███▀ ██ ██  ██  ██▄ ▀████ 
                                                                          ██ 
                                                                        ▀▀▀  
APAGAR TUDO ISSO QUANDO CONCLUIR O CONFIG!!!!!!!!!!!! ------------------------*/

static ServerConfig makeServer8080()
{
	ServerConfig server;

	server._port = 8080;
	server._host = "127.0.0.1";
	server._name = "127.0.0.1";
	server._root = "./www/";
	server._index = "index.html";
	server._max_body_size = 2048;

	server._error_pages[403] = "error_pages/403.html";
	server._error_pages[404] = "error_pages/404.html";
	server._error_pages[405] = "error_pages/405.html";
	server._error_pages[413] = "error_pages/413.html";
	server._error_pages[500] = "error_pages/500.html";

	LocationConfig root = makeLocation("/", false);
	root._methods.push_back("GET");
	root._methods.push_back("POST");
	root._methods.push_back("DELETE");
	server._locations.push_back(root);

	LocationConfig uploads = makeLocation("/uploads", true);
	uploads._methods.push_back("GET");
	uploads._methods.push_back("POST");
	uploads._methods.push_back("DELETE");
	server._locations.push_back(uploads);

	LocationConfig readonly = makeLocation("/readonly", true);
	readonly._methods.push_back("GET");
	server._locations.push_back(readonly);

	LocationConfig errorPages = makeLocation("/error_pages", true);
	errorPages._methods.push_back("GET");
	server._locations.push_back(errorPages);

	LocationConfig cgi = makeLocation("/cgi-bin", false);
	cgi._methods.push_back("GET");
	cgi._methods.push_back("POST");
	cgi._cgi_path = "/usr/bin/python3";
	cgi._cgi_extension = ".py";
	server._locations.push_back(cgi);

	return (server);
}

static ServerConfig makeServer8081()
{
	ServerConfig server;

	server._port = 8081;
	server._host = "127.0.0.1";
	server._name = "127.0.0.1";
	server._root = "./www/listener/";
	server._index = "index.html";
	server._max_body_size = 2048;

	LocationConfig root = makeLocation("/", false);
	root._methods.push_back("GET");
	root._methods.push_back("POST");
	root._methods.push_back("DELETE");
	server._locations.push_back(root);

	return (server);
}

Config makeConfig(std::string mocktype)
{
	Config config;

	if (mocktype == "MOCK_BASIC")
	{
		config._servers.push_back(makeServer8080());
		return (config);
	}

	if (mocktype == "MOCK_2SERVERS")
	{
		config._servers.push_back(makeServer8080());
		config._servers.push_back(makeServer8081());
		return (config);
	}

	return (config);
}

/*
                        ▀▀                                          
 ▀▀█▄ ████▄ ▄████ ██ ██ ██ ██ ██ ▄███▄   ▄████ ██ ██ ▄█▀█▄          
▄█▀██ ██ ▀▀ ██ ██ ██ ██ ██ ██▄██ ██ ██   ██ ██ ██ ██ ██▄█▀          
▀█▄██ ██    ▀████ ▀██▀█ ██▄ ▀█▀  ▀███▀   ▀████ ▀██▀█ ▀█▄▄▄          
               ██                           ██                      
               ▀▀                           ▀▀                      
                                                                    
                                                                    
                                                             ▄▄     
▄████ ▄█▀█▄ ████▄ ▄███▄ ██ ██   ▄███▄   ███▄███▄ ▄███▄ ▄████ ██ ▄█▀ 
██ ██ ██▄█▀ ██ ▀▀ ██ ██ ██ ██   ██ ██   ██ ██ ██ ██ ██ ██    ████   
▀████ ▀█▄▄▄ ██    ▀███▀ ▀██▀█   ▀███▀   ██ ██ ██ ▀███▀ ▀████ ██ ▀█▄ 
   ██                                                               
 ▀▀▀                                                                		  */

// ARQUIVO CONFIG 1 //////////////////////////////////
// copiei o default do Pedro

// server {
//     listen 8080;
//     server_name 127.0.0.1;
//     root ./www/;
//     index index.html;
//     client_max_body_size 2k;

//     error_page 403 error_pages/403.html;
//     error_page 404 error_pages/404.html;
//     error_page 405 error_pages/405.html;
//     error_page 413 error_pages/413.html;
//     error_page 500 error_pages/500.html;

//     location / {
//         allow_methods GET POST DELETE;
//         autoindex off;
//     }

//     location /uploads {
//         allow_methods GET POST DELETE;
//         autoindex on;
//     }

//     location /readonly {
//         allow_methods GET;
//         autoindex on;
//     }

//     location /error_pages {
//         allow_methods GET;
//         autoindex on;
//     }

//     location /go-home {
//         return 302 /;
//     }

//     location /cgi-bin {
//         allow_methods GET POST;
//         autoindex off;
//         cgi_path /usr/bin/python3;
//         cgi_ext .py;
//     }
// }


// ARQUIVO CONFIG 2 //////////////////////////////////
// o mesmo que o 1, mas com outro server, na porta 8081, e com root diferente


// server {
//     listen 8080;
//     server_name 127.0.0.1;
//     root ./www/;
//     index index.html;
//     client_max_body_size 2k;

//     error_page 403 error_pages/403.html;
//     error_page 404 error_pages/404.html;
//     error_page 405 error_pages/405.html;
//     error_page 413 error_pages/413.html;
//     error_page 500 error_pages/500.html;

//     location / {
//         allow_methods GET POST DELETE;
//         autoindex off;
//     }

//     location /uploads {
//         allow_methods GET POST DELETE;
//         autoindex on;
//     }

//     location /readonly {
//         allow_methods GET;
//         autoindex on;
//     }

//     location /error_pages {
//         allow_methods GET;
//         autoindex on;
//     }

//     location /go-home {
//         return 302 /;
//     }

//     location /cgi-bin {
//         allow_methods GET POST;
//         autoindex off;
//         cgi_path /usr/bin/python3;
//         cgi_ext .py;
//     }
// }
//
// server {
//     listen 8081;
//     server_name 127.0.0.1;
//     root ./www/listener/;
//     index index.html;
//     client_max_body_size 2k;

//        location / {
//         allow_methods GET POST DELETE;
//         autoindex off;
//     }
// }
