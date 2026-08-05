#include "Wkiti.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>

// --- PHASE 18: Initialize Static Performance Members ---
// These are shared across all Response objects to save memory and time
std::map<std::string, std::string> Wkiti::Response::template_cache;
std::mutex Wkiti::Response::cache_mutex;

// Phase 9: Helper to add a cookie to the response
void Wkiti::Response::set_cookie(std::string name, std::string value) {
    std::string cookie_str = name + "=" + value + "; Path=/; HttpOnly";
    cookies.push_back(cookie_str);
}

// PHASE 11: Dedicated JSON sender
void Wkiti::Response::json(nlohmann::json data) {
    this->headers["Content-Type"] = "application/json";
    this->body = data.dump(4);
}

// Phase 2, 6, 9, 11 & 18: Convert the Response object into a raw HTTP string
std::string Wkiti::Response::to_string() {
    std::string status_text;
    switch (status_code) {
        case 200: status_text = "OK"; break;
        case 201: status_text = "Created"; break;
        case 403: status_text = "Forbidden"; break;
        case 404: status_text = "Not Found"; break;
        case 500: status_text = "Internal Server Error"; break;
        default:  status_text = "OK"; break;
    }

    std::string res = "HTTP/1.1 " + std::to_string(status_code) + " " + status_text + "\r\n";

    if (headers.find("Content-Type") == headers.end()) {
        res += "Content-Type: text/html\r\n";
    }
    res += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    res += "Connection: close\r\n";

    for (auto const& [key, value] : headers) {
        res += key + ": " + value + "\r\n";
    }

    for (const auto& cookie : cookies) {
        res += "Set-Cookie: " + cookie + "\r\n";
    }

    res += "\r\n";
    res += body;

    return res;
}

// Phase 5 & 18: Optimized Template Engine with RAM Caching
void Wkiti::Response::render(std::string template_name, std::map<std::string, std::string> context) {
    std::string content;

    // --- PHASE 18 PERFORMANCE: CACHE CHECK ---
    {
        // We use a lock to ensure thread safety when reading/writing the global cache
        std::lock_guard<std::mutex> lock(cache_mutex);
        
        if (template_cache.count(template_name)) {
            // Found in RAM! 1000x faster than disk
            content = template_cache[template_name];
        } else {
            // Not in cache, read from disk once
            std::string path = "templates/" + template_name;
            std::ifstream file(path);

            if (!file.is_open()) {
                this->status_code = 500;
                this->body = "<h1>Internal Server Error</h1><p>Template not found: " + template_name + "</p>";
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            content = buffer.str();
            
            // Save to cache for the next user
            template_cache[template_name] = content;
            std::cout << "[PERFORMANCE]: Cached template '" << template_name << "' to RAM." << std::endl;
        }
    }

    // --- PHASE 5: Placeholder Replacement ---
    // Note: We replace placeholders AFTER getting content from cache 
    // because data (like username) changes for every user.
    for (auto const& [key, value] : context) {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = content.find(placeholder);
        while (pos != std::string::npos) {
            content.replace(pos, placeholder.length(), value);
            pos = content.find(placeholder, pos + value.length());
        }
    }

    this->body = content;
    this->headers["Content-Type"] = "text/html";
}