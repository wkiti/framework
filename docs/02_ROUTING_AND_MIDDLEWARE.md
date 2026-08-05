# 🌐 Routing, Middleware, & Crash Protection

The **Wkiti C++ Framework** provides a high-performance routing engine, an asynchronous middleware pipeline, and built-in crash protection.

---

## 1. HTTP Routing System

Wkiti maps incoming HTTP verbs and paths directly to C++ lambda functions or handler functions.

### Route Handler Signature
A handler function accepts a `const Wkiti::Request&` and a `Wkiti::Response&`:
```cpp
using Handler = std::function<void(const Request&, Response&)>;
Supported HTTP Methods

code<>
Wkiti::Server app;

// 1. GET (Retrieve Data / Render HTML)
app.get("/users", [](const Wkiti::Request& req, Wkiti::Response& res) {
    res.body = "<h1>User List</h1>";
});

// 2. POST (Create Data)
app.post("/api/users", [](const Wkiti::Request& req, Wkiti::Response& res) {
    res.status_code = 201; // Created
    res.json({{"status", "created"}});
});

// 3. PUT (Update Data)
app.put("/api/users", [](const Wkiti::Request& req, Wkiti::Response& res) {
    res.json({{"status", "updated"}});
});

// 4. DELETE (Remove Data - named 'del' because 'delete' is a C++ keyword)
app.del("/api/users", [](const Wkiti::Request& req, Wkiti::Response& res) {
    res.json({{"status", "deleted"}});
});

2. Request & Response Objects
Request Object (Wkiti::Request)
The Request struct encapsulates all data sent by the client browser or API consumer:

code<>
struct Request {
    std::string method;                          // "GET", "POST", etc.
    std::string path;                            // "/about", "/api/users"
    std::string version;                         // "HTTP/1.1"
    std::map<std::string, std::string> headers;  // Header key-value pairs
    std::string body;                            // Raw POST/PUT body data

    bool is_websocket() const;                   // Checks if Upgrade: websocket header exists
};
Response Object (Wkiti::Response)
The Response struct formats outgoing data to the client:

code<>
struct Response {
    int status_code = 200;                       // HTTP Status Code
    std::string body;                            // Output HTML/JSON/Text
    std::map<std::string, std::string> headers;  // Output Headers
    std::vector<std::string> cookies;            // Cookies list

    void set_cookie(std::string name, std::string value);
    void render(std::string template_name, std::map<std::string, std::string> context);
    void json(nlohmann::json data);
};

3. Middleware Architecture
Middlewares are functions that execute before a request reaches the route handler. Returning true continues the request chain; returning false aborts the request immediately.

code<>
// Middleware Signature
using Middleware = std::function<bool(const Request&, Response&)>;

Registering Global Middleware

code<>
// 1. Logging Middleware
app.use([](const Wkiti::Request& req, Wkiti::Response& res) {
    std::cout << "[LOG]: " << req.method << " " << req.path << std::endl;
    return true; // Continue execution
});

// 2. Security Headers Middleware
app.use([](const Wkiti::Request& req, Wkiti::Response& res) {
    res.headers["X-Content-Type-Options"] = "nosniff";
    res.headers["X-Frame-Options"] = "DENY";
    return true;
});

// 3. Access Control (Gatekeeper) Middleware
app.use([](const Wkiti::Request& req, Wkiti::Response& res) {
    if (req.path == "/forbidden") {
        res.status_code = 403;
        res.body = "<h1>403 Forbidden</h1><p>Blocked by Middleware</p>";
        return false; // ABORT request immediately
    }
    return true;
});

4. Custom Error Handling & Crash Protection
Registering Custom Error Pages
Instead of returning plain generic text, developers can register custom 404 and 500 error pages:

code<>
// Custom 404 Page Not Found
app.on_error(404, [](const Wkiti::Request& req, Wkiti::Response& res) {
    res.body = "<h1>404 Not Found</h1><p>The page " + req.path + " does not exist.</p>";
});

// Custom 500 Internal Server Error
app.on_error(500, [](const Wkiti::Request& req, Wkiti::Response& res) {
    res.body = "<h1>500 Internal Server Error</h1><p>Something went wrong.</p>";
});

Crash Protection (try-catch Isolation)
In C++, an uncaught exception inside a route handler (such as an out-of-bounds vector access or null pointer) normally terminates the entire process.
Wkiti wraps every request execution in a try-catch block inside worker threads:

code<>
try {
    routes[route_key](req, res);
} catch (const std::exception& e) {
    std::cerr << "[CRITICAL EXCEPTION]: " << e.what() << std::endl;
    trigger_error(500, req, res); // Catches the crash gracefully
}

If a route throws an exception, the user receives a 500 Internal Server Error page, and the server remains online for all other users.