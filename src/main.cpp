#include "Wkiti.hpp"
#include <iostream>
#include <vector>
#include <chrono>   
#include <thread>   

// Shortcut for the JSON library
using json = nlohmann::json;

int main() {
    // 1. Initialize the Server and Database
    Wkiti::Server app;
    Wkiti::Database db("wkiti_app.db");

    // 2. PHASE 8: Initialize ORM
    Wkiti::Model::setDatabase(&db);

    // 3. PHASE 9: Database Schema
    db.execute("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT, "
               "email TEXT, "
               "password TEXT);");

    // 4. PHASE 16: ACTIVATE HTTPS
    app.use_https("cert.pem", "key.pem");

    // --- PHASE 6 & 13: MIDDLEWARE ---
    app.use([](const Wkiti::Request& req, Wkiti::Response& res) {
        std::cout << "[SECURE THREAD " << std::this_thread::get_id() << "]: " 
                  << req.method << " " << req.path << std::endl;
        return true; 
    });

    app.use([](const Wkiti::Request& req, Wkiti::Response& res) {
        res.headers["X-Framework-Status"] = "Wkiti-v1.0-Production";
        res.headers["Strict-Transport-Security"] = "max-age=31536000; includeSubDomains"; 
        return true;
    });

    // --- PHASE 4: STATIC FILES ---
    app.static_files("public");

    // --- VERSION 1.0: CUSTOM ERROR PAGES ---
    
    // Custom 404 (Not Found)
    app.on_error(404, [](const Wkiti::Request& req, Wkiti::Response& res) {
        res.body = "<html><body style='font-family:sans-serif; text-align:center; margin-top:50px;'>"
                   "<h1 style='color:red;'>404 - Page Not Found</h1>"
                   "<p>Sorry, the page <b>" + Wkiti::Security::escape_html(req.path) + "</b> does not exist.</p>"
                   "<br><a href='/'>Go back Home</a>"
                   "</body></html>";
    });

    // Custom 500 (Internal Server Error)
    app.on_error(500, [](const Wkiti::Request& req, Wkiti::Response& res) {
        res.body = "<html><body style='font-family:sans-serif; text-align:center; margin-top:50px;'>"
                   "<h1>500 - Server Error</h1>"
                   "<p>Our engineers have been notified. Please try again later.</p>"
                   "</body></html>";
    });

    // --- VERSION 1.0: CRASH SIMULATION ROUTE ---
    app.get("/crash", [](const Wkiti::Request& req, Wkiti::Response& res) {
        std::cout << "Simulating a C++ code crash..." << std::endl;
        std::vector<int> empty_list;
        // This will instantly throw a std::out_of_range exception!
        int x = empty_list.at(100); 
        res.body = "You will never see this text because the line above crashed.";
    });

    // --- PHASE 13: CONCURRENCY TEST ---
    app.get("/slow", [](const Wkiti::Request& req, Wkiti::Response& res) {
        std::cout << "Handling slow request securely..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        res.body = "<h1>Secure Slow Task Complete</h1><p>Processed on an encrypted background thread.</p>";
    });

    // --- PHASE 9 & 14 & 1.0: SECURE AUTHENTICATION ---
    app.get("/register", [](const Wkiti::Request& req, Wkiti::Response& res) {
        Wkiti::User u;
        u.name = "Ali";
        u.email = "ali@example.com";
        u.password = "secret123"; 
        u.save();

        res.body = "<h1>Registration Successful (Encrypted)</h1>"
                   "<p>Your password was hashed and stored securely.</p>"
                   "<a href='/login'>Secure Login</a>";
    });

    app.get("/login", [](const Wkiti::Request& req, Wkiti::Response& res) {
        Wkiti::User user = Wkiti::User::find_by_email("ali@example.com");

        std::string attempt_hash = Wkiti::Security::hash_password("secret123");

        if (user.email == "ali@example.com" && user.password == attempt_hash) {
            res.set_cookie("session_id", "secure_12345");
            res.body = "<h1>Secure Login Successful!</h1><a href='/dashboard'>Go to Dashboard</a>";
        } else {
            res.status_code = 403;
            res.body = "<h1>Login Failed</h1>";
        }
    });

    app.get("/dashboard", [](const Wkiti::Request& req, Wkiti::Response& res) {
        bool logged_in = false;
        if (req.headers.count("Cookie") && req.headers.at("Cookie").find("session_id=secure_12345") != std::string::npos) {
            logged_in = true;
        }

        if (logged_in) {
            res.body = "<h1>Secure Dashboard</h1><p>You are viewing this over an encrypted TLS connection.</p><a href='/logout'>Logout</a>";
        } else {
            res.status_code = 403;
            res.body = "<h1>Access Denied</h1><a href='/login'>Login</a>";
        }
    });

    app.get("/logout", [](const Wkiti::Request& req, Wkiti::Response& res) {
        res.set_cookie("session_id", "deleted; Max-Age=0");
        res.body = "<h1>Logged Out</h1><a href='/'>Return Home</a>";
    });

    // --- PHASE 10 & 11: SECURE REST API ---
    app.get("/api/users", [](const Wkiti::Request& req, Wkiti::Response& res) {
        auto userList = Wkiti::User::all();
        json response_json = json::array();
        for (const auto& u : userList) {
            response_json.push_back({{"id", u.id}, {"name", u.name}, {"email", u.email}});
        }
        res.json(response_json);
    });

    app.post("/api/users", [](const Wkiti::Request& req, Wkiti::Response& res) {
        Wkiti::User u;
        u.name = "Secure Robot";
        u.email = "bot@secure.com";
        u.password = "api_pass";
        u.save();
        res.status_code = 201; 
        res.json({{"status", "success"}, {"encrypted", true}});
    });

    // --- VERSION 1.0: LIVE BENCHMARKING STATS ---
    app.get("/api/stats", [&app](const Wkiti::Request& req, Wkiti::Response& res) {
        long long reqs = app.get_total_requests();
        long long micros = app.get_total_microseconds();
        
        double avg_latency_ms = 0.0;
        if (reqs > 0) {
            avg_latency_ms = (double)micros / reqs / 1000.0; // Convert to milliseconds
        }

        json stats = {
            {"framework", "Wkiti C++"},
            {"version", "1.0"},
            {"total_requests_handled", reqs},
            {"total_processing_time_us", micros},
            {"average_latency_ms", avg_latency_ms},
            {"status", "High Performance Thread-Pool Active"}
        };

        res.json(stats);
    });

    // --- PHASE 3 & 5: HOME ---
    app.get("/", [](const Wkiti::Request& req, Wkiti::Response& res) {
        res.render("index.html", {
            {"title", "Wkiti Secure Framework"},
            {"username", "Developer"},
            {"status", "Version 1.0 is ACTIVE"}
        });
    });

    // 5. Start the Secure Server on port 8443
    // We explicitly tell it to use 4 Threads for the pool
    app.listen(8443, 4);

    return 0;
}