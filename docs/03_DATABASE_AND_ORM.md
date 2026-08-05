# 🗄️ Database & Object-Relational Mapping (ORM)

Wkiti includes a lightweight, secure C++ ORM and a **Multi-Database Driver Engine** supporting **SQLite3**, **PostgreSQL**, and **MySQL**.

---

## 1. Multi-Database Drivers & Connection URIs

Wkiti automatically detects the target database engine from the connection string URI.

### Connecting to SQLite, PostgreSQL, or MySQL

```cpp
#include "Wkiti.hpp"

// 1. SQLite Engine (File-based or sqlite:// prefix)
Wkiti::Database sqlite_db("wkiti_app.db");

// 2. PostgreSQL Engine (postgresql:// or postgres:// URI)
Wkiti::Database pg_db("postgresql://admin:secret@localhost:5432/production_db");

// 3. MySQL / MariaDB Engine (mysql:// URI)
Wkiti::Database mysql_db("mysql://root:secret@127.0.0.1:3306/app_db");

// Link your active database instance to the ORM
Wkiti::Model::setDatabase(&sqlite_db); // Works identically with pg_db or mysql_db

2. Raw Schema Execution (DDL)
Use execute() for DDL operations like CREATE TABLE or DROP TABLE:

Code<>
// DDL Execution across drivers
db.execute("CREATE TABLE IF NOT EXISTS users ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "name TEXT, "
           "email TEXT, "
           "password TEXT);");

3. SQL Parameter Binding & Placeholder Normalization
To prevent SQL Injection attacks, Wkiti uses prepared statements with parameterized inputs.

Automatic Placeholder Normalization (? →→ $1, $2)
PostgreSQL requires $1, $2 parameter placeholders instead of ?. Wkiti automatically translates ? to $1, $2 when connected to PostgreSQL, keeping your C++ code 100% portable across SQLite, Postgres, and MySQL!

code<>
// SAFE: Parameterized Execution across SQLite, Postgres, and MySQL
std::string sql = "INSERT INTO users (name, email) VALUES (?, ?);";
db.execute(sql, { "Ali", "ali@example.com" });

// SAFE: Parameterized Query
std::string query_sql = "SELECT * FROM users WHERE email = ? AND status = ?;";
auto results = db.query(query_sql, { "ali@example.com", "active" });

4. The ORM Engine (Wkiti::Model)
All database models inherit from the Wkiti::Model base class and implement toMap() and fromMap() serialization.
Defining a Model (User)

code<>
class User : public Wkiti::Model {
public:
    std::string name;
    std::string email;
    std::string password;

    // ORM CRUD Operations
    static std::vector<User> all();
    static User find(int id);
    static User find_by_email(std::string email);
    void save();

    // Serialization Mapping Logic
    std::map<std::string, std::string> toMap() override;
    void fromMap(std::map<std::string, std::string> data) override;
};

5. ORM Usage Examples
Creating and Saving Records (user.save())
When calling .save(), the model automatically hashes passwords using OpenSSL SHA-256 before inserting records:

code<>
Wkiti::User newUser;
newUser.name = "John Doe";
newUser.email = "john@example.com";
newUser.password = "mySecretPassword123";

newUser.save(); // Password is SHA-256 hashed and fields are securely bound
Fetching All Records (User::all())
code
C++
app.get("/users", [](const Wkiti::Request& req, Wkiti::Response& res) {
    std::vector<Wkiti::User> users = Wkiti::User::all();

    std::string html = "<h1>User Directory</h1><ul>";
    for (auto& user : users) {
        html += "<li>" + user.name + " (" + user.email + ")</li>";
    }
    html += "</ul>";
    
    res.body = html;
});

Finding Records (User::find_by_email())

code<>
Wkiti::User user = Wkiti::User::find_by_email("john@example.com");

if (!user.email.empty()) {
    std::cout << "User found: " << user.name << " (ID: " << user.id << ")" << std::endl;
}