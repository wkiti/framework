#include "Wkiti.hpp"
#include <iostream>

// 1. Initialize the static database pointer to null
Wkiti::Database* Wkiti::Model::db_ptr = nullptr;

// 2. Set the shared database connection for all models
void Wkiti::Model::setDatabase(Database* db) {
    db_ptr = db;
}

// --- User Model Implementation ---

// Maps C++ object fields to a generic map
std::map<std::string, std::string> Wkiti::User::toMap() {
    return { 
        {"name", name}, 
        {"email", email},
        {"password", password} 
    };
}

// Maps a database row back into C++ fields
void Wkiti::User::fromMap(std::map<std::string, std::string> data) {
    if (data.count("id"))       id = std::stoi(data["id"]);
    if (data.count("name"))     name = data["name"];
    if (data.count("email"))    email = data["email"];
    if (data.count("password")) password = data["password"];
}

// ORM: Fetch all users (Maintained)
std::vector<Wkiti::User> Wkiti::User::all() {
    std::vector<User> users;
    if (!db_ptr) return users;

    auto rows = db_ptr->query("SELECT * FROM users;");
    for (auto const& row : rows) {
        User u;
        u.fromMap(row);
        users.push_back(u);
    }
    return users;
}

// --- VERSION 1.0 UPGRADE: SECURE FIND BY ID ---
Wkiti::User Wkiti::User::find(int id) {
    Wkiti::User u;
    if (!db_ptr) return u;

    // Use '?' placeholder instead of string concatenation
    std::string sql = "SELECT * FROM users WHERE id = ? LIMIT 1;";
    auto rows = db_ptr->query(sql, { std::to_string(id) });
    
    if (!rows.empty()) {
        u.fromMap(rows[0]);
    }
    return u;
}

// --- VERSION 1.0 UPGRADE: SECURE FIND BY EMAIL ---
Wkiti::User Wkiti::User::find_by_email(std::string email) {
    Wkiti::User u;
    if (!db_ptr) return u;

    // Use '?' placeholder for the email string
    std::string sql = "SELECT * FROM users WHERE email = ? LIMIT 1;";
    auto rows = db_ptr->query(sql, { email });
    
    if (!rows.empty()) {
        u.fromMap(rows[0]);
    }
    return u;
}

// --- VERSION 1.0 UPGRADE: SECURE SAVE WITH AUTOMATIC HASHING ---
void Wkiti::User::save() {
    if (!db_ptr) return;

    // MILESTONE 1.0 FIX: 
    // We must hash the password BEFORE saving to the database.
    // This ensures the Login logic (which checks hashes) will work.
    std::string hashed_password = Security::hash_password(password);

    // Prepared Statement handles all escaping automatically.
    std::string sql = "INSERT INTO users (name, email, password) VALUES (?, ?, ?);";
    
    // Bind the data. Note we use the 'hashed_password' here.
    db_ptr->execute(sql, { name, email, hashed_password });

    std::cout << "[ORM]: User '" << email << "' saved with SHA-256 hashing." << std::endl;
}