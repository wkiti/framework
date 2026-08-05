#include "Wkiti.hpp"
#include <sstream>
#include <algorithm>

// PHASE 17 FIX: Helper function to clean strings
// Removes '\r', '\n' and leading/trailing spaces
std::string trim_string(const std::string& s) {
    std::string res = s;
    // Remove carriage returns and newlines
    res.erase(std::remove(res.begin(), res.end(), '\r'), res.end());
    res.erase(std::remove(res.begin(), res.end(), '\n'), res.end());
    
    // Remove leading and trailing spaces
    size_t first = res.find_first_not_of(' ');
    if (std::string::npos == first) return "";
    size_t last = res.find_last_not_of(' ');
    return res.substr(first, (last - first + 1));
}

Wkiti::Request Wkiti::Request::parse(std::string raw_data) {
    Request req;
    std::istringstream stream(raw_data);
    std::string line;

    // 1. Parse Request Line: METHOD PATH VERSION
    if (std::getline(stream, line)) {
        std::istringstream first_line(line);
        first_line >> req.method >> req.path >> req.version;
    }

    // 2. Parse Headers
    while (std::getline(stream, line)) {
        line = trim_string(line);
        
        // HTTP headers end with a blank line
        if (line.empty()) break; 

        auto colon = line.find(':');
        if (colon != std::string::npos) {
            // Extract and clean the Key and Value
            std::string key = trim_string(line.substr(0, colon));
            std::string value = trim_string(line.substr(colon + 1));
            req.headers[key] = value;
        }
    }

    // 3. Parse Body (The remaining data in the stream)
    // Important for POST/PUT requests
    std::string body_content;
    char ch;
    while (stream.get(ch)) {
        body_content += ch;
    }
    req.body = body_content;

    return req;
}