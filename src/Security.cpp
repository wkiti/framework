#include "Wkiti.hpp"
#include <openssl/evp.h> // Vetted Cryptography API
#include <iomanip>
#include <sstream>

// Phase 14: Prevent SQL Injection (Maintained)
std::string Wkiti::Security::escape_sql(std::string input) {
    std::string output = "";
    for (char c : input) {
        if (c == '\'') output += "''"; // Escape single quotes for SQLite
        else output += c;
    }
    return output;
}

// Phase 14: Prevent XSS (Maintained)
std::string Wkiti::Security::escape_html(std::string input) {
    std::string output = "";
    for (char c : input) {
        switch (c) {
            case '<':  output += "&lt;"; break;
            case '>':  output += "&gt;"; break;
            case '&':  output += "&amp;"; break;
            case '"':  output += "&quot;"; break;
            case '\'': output += "&#39;"; break;
            default:   output += c;
        }
    }
    return output;
}

// --- VERSION 1.0 UPGRADE: Secure SHA-256 Hashing ---
// This replaces the previous XOR-based obfuscation with industry-standard encryption.
std::string Wkiti::Security::hash_password(std::string password) {
    // 1. Framework Salt (Ensures identical passwords across different frameworks have different hashes)
    std::string salt = "WKITI_V1_STABLE_PROTECTION_SALT";
    std::string salted_input = password + salt;

    // 2. Setup OpenSSL Digest variables
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    // Create a Message Digest Context
    EVP_MD_CTX* context = EVP_MD_CTX_new();

    // 3. Initialize, Update (Process data), and Finalize the SHA-256 Hash
    if (context != nullptr) {
        EVP_DigestInit_ex(context, EVP_sha256(), nullptr);
        EVP_DigestUpdate(context, salted_input.c_str(), salted_input.length());
        EVP_DigestFinal_ex(context, hash, &hash_len);
        EVP_MD_CTX_free(context);
    } else {
        return "HASHING_CONTEXT_ERROR";
    }

    // 4. Convert the binary hash result into a readable Hexadecimal string
    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}