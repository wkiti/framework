========================================================================
                      WKITI C++ WEB FRAMEWORK v1.0
          High-Performance, Secure, Multi-Threaded C++ Backend
========================================================================

Wkiti C++ is a lightweight, full-stack web framework built from scratch 
in C++17. It features a thread-pool architecture, built-in ORM with 
Prepared Statement parameter binding, SHA-256 security hashing, 
OpenSSL TLS 1.3 encryption, a RAM-cached template engine, JSON API support, 
multi-database support (SQLite, PostgreSQL, MySQL), and an automated CLI.

------------------------------------------------------------------------
1. EXACT DIRECTORY STRUCTURE
------------------------------------------------------------------------

Wkiti-v1/
├── cert.pem                   <-- SSL Certificate (Phase 16)
├── key.pem                    <-- SSL Private Key (Phase 16)
├── wkiti_app.db               <-- SQLite Database File
├── wkiti_app.exe              <-- Compiled Server Executable
├── wkiti-cli.exe              <-- Compiled CLI Executable
├── framework_tests.exe        <-- Compiled Test Executable
├── compile and run.txt        <-- Build Script Helper
├── Document 5.pdf             <-- Reference Documentation
├── docs/                      <-- Framework Documentation Directory
├── include/
│   ├── Wkiti.hpp              <-- Master Framework Header
│   ├── sqlite3.h              <-- SQLite Header
│   └── nlohmann/
│       └── json.hpp           <-- JSON Library Header
├── src/
│   ├── cli.cpp                <-- CLI Tool Source Code
│   ├── CMakeLists.txt         <-- CMake Build Script
│   ├── Database.cpp           <-- Database Driver & Prepared Statements
│   ├── main.cpp               <-- Web Application Entry Point
│   ├── Model.cpp              <-- ORM Logic & Models
│   ├── Request.cpp            <-- HTTP Request Parser
│   ├── Response.cpp           <-- HTTP Response Generator & RAM Cache
│   ├── Security.cpp           <-- SHA-256 Hashing & Escaping
│   ├── Server.cpp             <-- Server, ThreadPool & TLS Engine
│   ├── sqlite3.c              <-- SQLite C Source
│   └── sqlite3.o              <-- Compiled SQLite Object
├── public/
│   ├── style.css              <-- Static Stylesheet
│   └── test.html              <-- Static HTML Test Page
├── templates/
│   ├── about.html             <-- Template Page
│   └── index.html             <-- Home Template Page
├── tests/
│   └── test_runner.cpp        <-- Automated System Tests
└── MyNewApp/                  <-- Generated Project Instance (via CLI)
    ├── include/               <-- Generated Self-Contained Headers
    ├── public/                <-- Generated Assets
    ├── src/                   <-- Generated Source Code
    └── templates/             <-- Generated HTML Templates


------------------------------------------------------------------------
2. PREREQUISITES & DEPENDENCIES
------------------------------------------------------------------------

- Compiler: C++17 compliant compiler (g++ 8+ or clang)
- Platform: Windows 10/11 (MinGW-w64 / MSYS2 UCRT64), Linux, or macOS
- Networking & Security:
  * Winsock2 (-lws2_32) [Windows Networking]
  * OpenSSL (-lssl -lcrypto -lcrypt32) [HTTPS TLS 1.3 Encryption]
  * nlohmann/json (Header-only, included in include/nlohmann/json.hpp)
- Database Drivers:
  * SQLite3 (Embedded default, included in src/sqlite3.o)
  * PostgreSQL (Optional: libpq / -lpq)
  * MySQL / MariaDB (Optional: libmysqlclient / -lmysqlclient)


------------------------------------------------------------------------
3. COMPILING AND RUNNING WKITI
------------------------------------------------------------------------

Open your PowerShell terminal inside the Wkiti-v1 directory:

1. Stop any running server:
   Stop-Process -Name "wkiti_app" -Force

2. Compile Wkiti Framework (Optimized Release Build):
   g++ -O3 -std=c++17 -I./include -I"C:/msys64/ucrt64/include" `
   src/main.cpp src/Server.cpp src/Database.cpp src/Model.cpp src/Request.cpp src/Response.cpp src/Security.cpp src/sqlite3.o `
   -o wkiti_app.exe `
   -L"C:/msys64/ucrt64/lib" -lws2_32 -lssl -lcrypto -lcrypt32

3. Launch Wkiti:
   ./wkiti_app.exe

4. Open Browser:
   https://localhost:8443


------------------------------------------------------------------------
4. CLI TOOL USAGE
------------------------------------------------------------------------

Wkiti includes a CLI generator tool to scaffold standalone projects.

1. Compile the CLI tool:
   g++ -std=c++17 src/cli.cpp -o wkiti-cli.exe

2. Create a new project:
   ./wkiti-cli.exe create MyNewApp

This automatically generates the 'MyNewApp' folder with all Wkiti headers, 
sources, templates, public assets, and starter main.cpp self-contained.


------------------------------------------------------------------------
5. FEATURE QUICKSTART & CODE EXAMPLES
------------------------------------------------------------------------

--- A. BASIC SERVER & ROUTING ---
#include "Wkiti.hpp"

int main() {
    Wkiti::Server app;

    app.get("/", [](const Wkiti::Request& req, Wkiti::Response& res) {
        res.body = "<h1>Welcome to Wkiti C++!</h1>";
    });

    app.listen(8443);
}


--- B. TEMPLATES & STATIC FILES ---
// Set folder for CSS/JS/Images
app.static_files("public");

// Render templates/index.html with RAM caching
app.get("/home", [](const auto& req, auto& res) {
    res.render("index.html", {
        {"title", "Home Page"},
        {"username", "Ali"}
    });
});


--- C. DATABASE SYSTEM & ORM (SQLITE, POSTGRESQL, MYSQL) ---

Wkiti supports multiple database engines through a unified ORM API.

1. SQLite (Embedded File-based Default):
   Wkiti::Database db("wkiti_app.db");

2. PostgreSQL Connection (Enterprise Driver):
   Wkiti::Database db("postgresql://user:password@localhost:5432/mydb");

3. MySQL / MariaDB Connection (Scalable Driver):
   Wkiti::Database db("mysql://user:password@localhost:3306/mydb");

// Link DB Engine to ORM
Wkiti::Model::setDatabase(&db);

// Table Creation
db.execute("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, email TEXT, password TEXT);");

// Unified ORM Usage (Works identically across SQLite, PostgreSQL, & MySQL)
Wkiti::User u;
u.name = "Ali";
u.email = "ali@example.com";
u.password = "secret123";
u.save(); // Uses Prepared Statement Parameter Binding and Auto-Hashes Password

// Fetch all users via ORM
auto users = Wkiti::User::all();

// Query by unique attribute
Wkiti::User user = Wkiti::User::find_by_email("ali@example.com");


--- D. AUTHENTICATION & COOKIES ---
app.get("/login", [](const auto& req, auto& res) {
    Wkiti::User user = Wkiti::User::find_by_email("ali@example.com");
    std::string typed_hash = Wkiti::Security::hash_password("secret123");

    if (user.password == typed_hash) {
        res.set_cookie("session_id", "secure_token_123");
        res.body = "<h1>Logged In!</h1>";
    } else {
        res.status_code = 403;
        res.body = "<h1>Login Failed</h1>";
    }
});


--- E. REST API & JSON ---
app.get("/api/users", [](const auto& req, auto& res) {
    auto users = Wkiti::User::all();
    nlohmann::json response_data = nlohmann::json::array();

    for (const auto& u : users) {
        response_data.push_back({{"id", u.id}, {"name", u.name}, {"email", u.email}});
    }

    res.json(response_data); // Sets Content-Type: application/json
});


--- F. CUSTOM ERROR HANDLING & CRASH PROTECTION ---
app.on_error(404, [](const auto& req, auto& res) {
    res.body = "<h1>404 - Page Not Found</h1>";
});

app.on_error(500, [](const auto& req, auto& res) {
    res.body = "<h1>500 - Server Internal Error</h1>";
});


--- G. HTTPS / TLS ENCRYPTION ---
// Generate certificates first:
// openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -nodes -subj "/CN=localhost"

app.use_https("cert.pem", "key.pem");
app.listen(8443, 4); // Port 8443 with 4 Worker Threads


------------------------------------------------------------------------
6. AUTOMATED TESTING
------------------------------------------------------------------------

Wkiti includes a dedicated test runner to verify core framework features 
without opening a browser.

1. Compile the Test Suite:
   g++ -std=c++17 -I./include -I"C:/msys64/ucrt64/include" `
   tests/test_runner.cpp src/Request.cpp src/Response.cpp src/Security.cpp `
   -o framework_tests.exe `
   -L"C:/msys64/ucrt64/lib" -lssl -lcrypto

2. Execute Tests:
   ./framework_tests.exe


------------------------------------------------------------------------
7. BENCHMARKING & METRICS
------------------------------------------------------------------------

Wkiti tracks internal request latency using high-resolution timers and 
atomic counters.

Access real-time stats endpoint:
GET https://localhost:8443/api/stats

Example JSON Output:
{
    "framework": "Wkiti C++",
    "version": "1.0",
    "total_requests_handled": 1500,
    "total_processing_time_us": 450000,
    "average_latency_ms": 0.3
}

------------------------------------------------------------------------
8. Complete C++ API Reference
------------------------------------------------------------------------

Class / Function	              Signature	                                   Description

Wkiti::Server::get()	          void get(string path, Handler h)	           Register GET route
Wkiti::Server::post()	          void post(string path, Handler h)            Register POST route
Wkiti::Server::put()	          void put(string path, Handler h)	           Register PUT route
Wkiti::Server::del()	          void del(string path, Handler h)	           Register DELETE route
Wkiti::Server::use()	          void use(Middleware m)	                   Add global middleware
Wkiti::Server::on_error()	      void on_error(int code, Handler h)	       Register custom 404/500 page
Wkiti::Server::use_https()	      void use_https(cert, key)	                   Enable TLS 1.3 Encryption
Wkiti::Server::listen()	          void listen(int port, int threads)     	   Launch server with ThreadPool
Wkiti::Response::render()	      void render(file, context)	               Render HTML template with RAM Cache
Wkiti::Response::json()	          void json(nlohmann::json data)	           Return formatted JSON API response
Wkiti::Response::set_cookie()     void set_cookie(name, value)	               Send HttpOnly session cookie
Wkiti::Database::execute()	      bool execute(sql, params)	                   Execute query using Prepared Statements
Wkiti::Database::query()	      vector<map> query(sql, params)	           Fetch rows using Prepared Statements
Wkiti::Security::hash_password()  string hash_password(str)	                   Hash password using OpenSSL SHA-256
Wkiti::Security::escape_html()	  string escape_html(str)	                   Escapes HTML characters (XSS Shield)


========================================================================
                     BUILD WITH WKITI C++ v1.0 🚀
========================================================================