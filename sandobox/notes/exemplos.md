# Exemplos de Requisições CGI

## Example 1: GET request to CGI

Imagine the browser/curl sends this to your webserv:

```
GET /cgi-bin/hello.py?name=Eduardo&lang=en HTTP/1.1
Host: localhost:8080
User-Agent: curl/8.5.0
Accept: */*
```

This is the real HTTP request received by the webserver.

The router should understand:

```
/cgi-bin/hello.py
```

is a CGI script, probably because of:

```
.py -> /usr/bin/python3
```

So the object you receive could look like this:

```cpp
CgiRequest request;

request.setMethod("GET");
request.setUri("/cgi-bin/hello.py");
request.setQueryString("name=Eduardo&lang=en");
request.setBody("");

request.setScriptPath("/home/eduardo/webserv/www/cgi-bin/hello.py");
request.setInterpreterPath("/usr/bin/python3");
request.setWorkingDirectory("/home/eduardo/webserv/www/cgi-bin");

request.setServerProtocol("HTTP/1.1");
request.setServerName("localhost");
request.setServerPort("8080");

request.setHeader("Host", "localhost:8080");
request.setHeader("User-Agent", "curl/8.5.0");
request.setHeader("Accept", "*/*");
```

Then your CGI environment would include things like:

```
REQUEST_METHOD=GET
QUERY_STRING=name=Eduardo&lang=en
SCRIPT_FILENAME=/home/eduardo/webserv/www/cgi-bin/hello.py
SERVER_PROTOCOL=HTTP/1.1
SERVER_NAME=localhost
SERVER_PORT=8080
HTTP_HOST=localhost:8080
HTTP_USER_AGENT=curl/8.5.0
```

For GET, the body is empty. The arguments are in the query string.

---

## Example 2: POST request to CGI

Now imagine a form submits data:

```
POST /cgi-bin/login.py HTTP/1.1
Host: localhost:8080
User-Agent: curl/8.5.0
Content-Type: application/x-www-form-urlencoded
Content-Length: 29

username=eduardo&password=123
```

This is the real HTTP request received by the webserver.

The object you receive should look like:

```cpp
CgiRequest request;

request.setMethod("POST");
request.setUri("/cgi-bin/login.py");
request.setQueryString("");
request.setBody("username=eduardo&password=123");

request.setScriptPath("/home/eduardo/webserv/www/cgi-bin/login.py");
request.setInterpreterPath("/usr/bin/python3");
request.setWorkingDirectory("/home/eduardo/webserv/www/cgi-bin");

request.setServerProtocol("HTTP/1.1");
request.setServerName("localhost");
request.setServerPort("8080");

request.setHeader("Host", "localhost:8080");
request.setHeader("User-Agent", "curl/8.5.0");
request.setHeader("Content-Type", "application/x-www-form-urlencoded");
request.setHeader("Content-Length", "29");
```

Then your CGI environment would include:

```
REQUEST_METHOD=POST
QUERY_STRING=
CONTENT_TYPE=application/x-www-form-urlencoded
CONTENT_LENGTH=29
SCRIPT_FILENAME=/home/eduardo/webserv/www/cgi-bin/login.py
SERVER_PROTOCOL=HTTP/1.1
SERVER_NAME=localhost
SERVER_PORT=8080
HTTP_HOST=localhost:8080
HTTP_USER_AGENT=curl/8.5.0
```

And your CGI module should send this to the CGI process through stdin:

```
username=eduardo&password=123
```

That is what the subject means when it says the full request and arguments must be available to CGI.

---

## Main difference between the two

**GET:**
- data usually comes from query string
- body is empty
- CGI reads `QUERY_STRING`

**POST:**
- data usually comes from body
- query string may be empty
- CGI reads stdin
- `CONTENT_LENGTH` tells CGI how much body exists

So your module does not need to care about raw HTTP. Your contract should be:

**Team gives you:**
- method
- path
- query string
- headers
- body
- script path
- interpreter path
- working directory

**You return:**
- status
- headers
- body

That is the clean CGI boundary.
</file_text>