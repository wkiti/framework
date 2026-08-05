# 🗄️ Database & Object-Relational Mapping (ORM)

Wkiti includes a lightweight, secure C++ ORM and a **Multi-Database Driver Engine** supporting **SQLite3**, **PostgreSQL**, and **MySQL**.

---

## 1. Installing Database C Client Libraries

Before connecting to external PostgreSQL or MySQL servers, install the client library headers on your operating system:

```bash

# Windows MSYS2 UCRT64
pacman -S mingw-w64-ucrt-x86_64-postgresql mingw-w64-ucrt-x86_64-libmariadbclient

# Ubuntu / Debian Linux
sudo apt update && sudo apt install -y libpq-dev libmariadb-dev

# macOS (Homebrew)
brew install postgresql mariadb-connector-c

2. Connection String Architecture in Wkiti
Wkiti encapsulates database connectivity inside Wkiti::Database. It automatically detects the database driver from the connection URI:

code<>
#include "Wkiti.hpp"

// 1. SQLite Engine (Embedded File)
Wkiti::Database sqlite_db("sqlite://wkiti_app.db");

// 2. PostgreSQL Engine (Enterprise Cluster)
Wkiti::Database pg_db("postgresql://postgres:secret@localhost:5432/wkiti_db");

// 3. MySQL / MariaDB Engine (Scalable Server)
Wkiti::Database mysql_db("mysql://root:secret@localhost:3306/wkiti_db");

// Link your active database instance to the ORM
Wkiti::Model::setDatabase(&sqlite_db); // Works identically with pg_db or mysql_db

3. Driver Integration Details
Under the hood, Database.cpp invokes native C client driver functions:
A. PostgreSQL Driver Integration (libpq)

code<>
#include <postgresql/libpq-fe.h>

// PostgreSQL Query Logic
std::vector<std::map<std::string, std::string>> query_postgres(PGconn* conn, std::string sql) {
    std::vector<std::map<std::string, std::string>> results;
    PGresult* res = PQexec(conn, sql.c_str());

    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        int rows = PQntuples(res);
        int cols = PQnfields(res);

        for (int i = 0; i < rows; i++) {
            std::map<std::string, std::string> row;
            for (int j = 0; j < cols; j++) {
                std::string col_name = PQfname(res, j);
                std::string val = PQgetvalue(res, i, j);
                row[col_name] = val;
            }
            results.push_back(row);
        }
    }
    PQclear(res);
    return results;
}

B. MySQL Driver Integration (libmysqlclient)

code<>
#include <mysql/mysql.h>

// MySQL Query Logic
std::vector<std::map<std::string, std::string>> query_mysql(MYSQL* conn, std::string sql) {
    std::vector<std::map<std::string, std::string>> results;
    if (mysql_query(conn, sql.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            int num_fields = mysql_num_fields(res);
            MYSQL_FIELD* fields = mysql_fetch_fields(res);
            MYSQL_ROW row;

            while ((row = mysql_fetch_row(res))) {
                std::map<std::string, std::string> r;
                for (int i = 0; i < num_fields; i++) {
                    std::string col_name = fields[i].name;
                    std::string val = row[i] ? row[i] : "NULL";
                    r[col_name] = val;
                }
                results.push_back(r);
            }
            mysql_free_result(res);
        }
    }
    return results;
}

4. SQL Parameter Binding & Placeholder Normalization (? →→ $1, $2)

To prevent SQL Injection attacks, Wkiti uses prepared statements with parameterized inputs.

Automatic Placeholder Translation
PostgreSQL requires $1, $2 parameter placeholders instead of ?. Wkiti automatically translates ? to $1, $2 when connected to PostgreSQL, keeping your C++ code 100% portable across SQLite, Postgres, and MySQL!

code<>

// SAFE: Parameterized Execution across SQLite, Postgres, and MySQL
std::string sql = "INSERT INTO users (name, email) VALUES (?, ?);";
db.execute(sql, { "Ali", "ali@example.com" });

// SAFE: Parameterized Query
std::string query_sql = "SELECT * FROM users WHERE email = ? AND status = ?;";
auto results = db.query(query_sql, { "ali@example.com", "active" });

5. Compiling with PostgreSQL and MySQL Support

When compiling Wkiti with PostgreSQL and MySQL support enabled, add -lpq and -lmariadb to your build command:

code Powershell
# Windows MSYS2 Compilation Command

g++ -O3 -std=c++17 -I./include -I"C:/msys64/ucrt64/include" `
src/main.cpp src/Server.cpp src/Database.cpp src/Model.cpp src/Request.cpp src/Response.cpp src/Security.cpp src/sqlite3.o `
-o wkiti_app.exe `
-L"C:/msys64/ucrt64/lib" -lws2_32 -lssl -lcrypto -lcrypt32 -lpq -lmariadb

6. The ORM Engine (Wkiti::Model)

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

7. ORM Usage Examples

Creating and Saving Records (user.save())
When calling .save(), the model automatically hashes passwords using OpenSSL SHA-256 before inserting records:

code<>

Wkiti::User newUser;
newUser.name = "John Doe";
newUser.email = "john@example.com";
newUser.password = "mySecretPassword123";

newUser.save(); // Password is SHA-256 hashed and fields are securely bound
Fetching All Records (User::all())

code<>

app.get("/users", [](const auto& req, auto& res) {
    std::vector<Wkiti::User> users = Wkiti::User::all(); // Managed ORM call
    res.json(users);
});

Finding Records (User::find_by_email())

code<>

Wkiti::User user = Wkiti::User::find_by_email("john@example.com");

if (!user.email.empty()) {
    std::cout << "User found: " << user.name << " (ID: " << user.id << ")" << std::endl;
}