// CgiRequestHandler.hpp
#include "IRequestHandler.hpp"
#include "InitCgiGateway.hpp"

class CgiRequestHandler : public IRequestHandler {
private:
    InitCgiGateway& _gateway;
    CgiRouteConfig  _config;

public:
    CgiRequestHandler(InitCgiGateway& gateway, const CgiRouteConfig& config) 
        : _gateway(gateway), _config(config) {}

    // Polymorphic implementation
    HttpResponse handle(const HttpRequest& request) /* override */ {
        // Implement Fabio's non-blocking start logic here
        // _gateway.start(...);
        // return an intermediate HttpResponse indicating processing, 
        // or defer until epoll finishes
    }
};