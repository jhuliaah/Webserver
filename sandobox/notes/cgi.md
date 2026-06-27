# Why Fabio is the Best Foundation

Fabio's architecture is almost practically begging for a polymorphic interface.

**Unified Signatures:** If you look at Fabio's static handlers (`GetRequestHandler`, `PostRequestHandler`, and `DeleteRequestHandler`), they all independently implement the exact same method signature: `HttpResponse handle(const HttpRequest& request)`.

**Dependency Injection:** Fabio already utilizes interface-like patterns for CGI, passing an `InitCgiGateway&` into the `CgiHandler` constructor.

## How to Implement Your Polymorphic Foundation

You can take Fabio's classes and easily unify them by creating a single, pure abstract C++98 interface. This will allow your router to hold a pointer to `IRequestHandler` and call `.handle()` without caring if it's dealing with a static file or a CGI script.

Here is the blueprint you can use to start copying and adapting Fabio's code:

### 1. Create the Base Interface

```cpp
// IRequestHandler.hpp
#ifndef IREQUESTHANDLER_HPP
#define IREQUESTHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class IRequestHandler {
public:
    virtual ~IRequestHandler() {}
    virtual HttpResponse handle(const HttpRequest& request) = 0;
};

#endif
```

### 2. Adapt Fabio's CGI Handler

You can adapt Fabio's `CgiHandler` to inherit from this interface. It will manage the non-blocking execution inside.

```cpp
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
```

### 3. Prepare for Jhulia's Static Files

If Jhulia struggles, you can effortlessly plug in a static handler using the exact same foundation, taking inspiration from Fabio's `GetRequestHandler`.

```cpp
// StaticRequestHandler.hpp
#include "IRequestHandler.hpp"

class StaticRequestHandler : public IRequestHandler {
public:
    HttpResponse handle(const HttpRequest& request) /* override */ {
        // Read file from disk and return 200 OK with HTML body
        // (Just like Fabio's GetRequestHandler does)
    }
};
```

## Summary Strategy

Copy Fabio's `CgiOrchestrator`, `CgiHandler`, and network event mechanics. Wrap the handler logic in your own `IRequestHandler` base class. If you do this, your Webserv router will be beautifully clean, dispatching both CGI and Static content dynamically.
</file_text>