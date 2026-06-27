// StaticRequestHandler.hpp
#include "IRequestHandler.hpp"

class StaticRequestHandler : public IRequestHandler
{
    public:
        HttpResponse handle(const HttpRequest& request) /* override */
        {
            // Read file from disk and return 200 OK with HTML body
            // (Just like Fabio's GetRequestHandler does)
        }
};
