#include "Wkiti.hpp"
#include <iostream>
#include <sstream>

Wkiti::Database::Database(std::string conn_str) : connection_string(conn_str) {
    if (conn_str.rfind("postgresql://", 0) == 0 || conn_str.rfind("postgres://", 0) == 0) {
        type = DatabaseType::POSTGRESQL;
        init_postgresql(conn_str);
    } else if (conn_str.rfind("mysql://", 0) == 0) {
        type = DatabaseType::MYSQL;
        init_mysql(conn_str);
    } else {
        type = DatabaseType::SQLITE;
        init_sqlite(conn_str);
    }
}

Wkiti::Database::~Database() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }

#ifdef WKITI_HAS_POSTGRES
    if (pg_conn) {
        PQfinish(pg_conn);
        pg_conn = nullptr;
    }
#endif

#ifdef WKITI_HAS_MYSQL
    if (mysql_conn) {
        mysql_close(mysql_conn);
        mysql_conn = nullptr;
    }
#endif
}

// 1. SQLite Driver
void Wkiti::Database::init_sqlite(const std::string& db_name) {
    std::string clean_name = db_name;
    if (clean_name.rfind("sqlite://", 0) == 0) {
        clean_name = clean_name.substr(9);
    }

    if (sqlite3_open(clean_name.c_str(), &db) != SQLITE_OK) {
        std::cerr << "SQLite Error: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "[DB]: Connected to SQLite -> " << clean_name << std::endl;
    }
}

// 2. PostgreSQL Native Driver (libpq)
void Wkiti::Database::init_postgresql(const std::string& conn_uri) {
#ifdef WKITI_HAS_POSTGRES
    pg_conn = PQconnectdb(conn_uri.c_str());
    if (PQstatus(pg_conn) != CONNECTION_OK) {
        std::cerr << "[POSTGRES ERROR]: Connection failed: " << PQerrorMessage(pg_conn) << std::endl;
    } else {
        std::cout << "[DB]: Connected to PostgreSQL via libpq -> " << conn_uri << std::endl;
    }
#else
    std::cout << "[DB]: Initialized PostgreSQL URI -> " << conn_uri << std::endl;
#endif
}

// 3. MySQL / MariaDB Native Driver (libmariadb / libmysqlclient)
void Wkiti::Database::init_mysql(const std::string& conn_uri) {
#ifdef WKITI_HAS_MYSQL
    mysql_conn = mysql_init(nullptr);
    std::cout << "[DB]: Connected to MySQL Engine -> " << conn_uri << std::endl;
#else
    std::cout << "[DB]: Initialized MySQL URI -> " << conn_uri << std::endl;
#endif
}

// Placeholder Normalization (? -> $1, $2)
std::string Wkiti::Database::normalize_sql_for_postgres(const std::string& sql) {
    std::string result;
    int param_idx = 1;
    for (char c : sql) {
        if (c == '?') {
            result += "$" + std::to_string(param_idx++);
        } else {
            result += c;
        }
    }
    return result;
}

// EXECUTE
bool Wkiti::Database::execute(std::string sql) {
    if (type == DatabaseType::SQLITE) {
        char* err = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, 0, &err) != SQLITE_OK) {
            std::cerr << "SQL Error: " << (err ? err : "Unknown") << std::endl;
            if (err) sqlite3_free(err);
            return false;
        }
        return true;
    } 
#ifdef WKITI_HAS_POSTGRES
    else if (type == DatabaseType::POSTGRESQL && pg_conn) {
        std::string pg_sql = normalize_sql_for_postgres(sql);
        PGresult* res = PQexec(pg_conn, pg_sql.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
            std::cerr << "[POSTGRES ERROR]: " << PQerrorMessage(pg_conn) << std::endl;
            PQclear(res);
            return false;
        }
        PQclear(res);
        return true;
    }
#endif
#ifdef WKITI_HAS_MYSQL
    else if (type == DatabaseType::MYSQL && mysql_conn) {
        if (mysql_query(mysql_conn, sql.c_str())) {
            std::cerr << "[MYSQL ERROR]: " << mysql_error(mysql_conn) << std::endl;
            return false;
        }
        return true;
    }
#endif
    return false;
}

bool Wkiti::Database::execute(std::string sql, const std::vector<std::string>& params) {
    if (type == DatabaseType::SQLITE) {
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "[DB 1.0 ERROR]: Prepare failed: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        for (int i = 0; i < (int)params.size(); ++i) {
            sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
        }

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return (rc == SQLITE_DONE || rc == SQLITE_ROW);
    }
    return execute(sql); // Fallback to string execution for drivers
}

// QUERY
static int db_callback(void* data, int argc, char** argv, char** colNames) {
    auto* results = static_cast<std::vector<std::map<std::string, std::string>>*>(data);
    std::map<std::string, std::string> row;
    for (int i = 0; i < argc; i++) {
        row[colNames[i]] = argv[i] ? argv[i] : "NULL";
    }
    results->push_back(row);
    return 0;
}

std::vector<std::map<std::string, std::string>> Wkiti::Database::query(std::string sql) {
    std::vector<std::map<std::string, std::string>> results;

    if (type == DatabaseType::SQLITE) {
        char* err = nullptr;
        if (sqlite3_exec(db, sql.c_str(), db_callback, &results, &err) != SQLITE_OK) {
            std::cerr << "SQL Error: " << (err ? err : "Unknown") << std::endl;
            if (err) sqlite3_free(err);
        }
    }
#ifdef WKITI_HAS_POSTGRES
    else if (type == DatabaseType::POSTGRESQL && pg_conn) {
        std::string pg_sql = normalize_sql_for_postgres(sql);
        PGresult* res = PQexec(pg_conn, pg_sql.c_str());
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
    }
#endif
#ifdef WKITI_HAS_MYSQL
    else if (type == DatabaseType::MYSQL && mysql_conn) {
        if (mysql_query(mysql_conn, sql.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(mysql_conn);
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
    }
#endif

    return results;
}

std::vector<std::map<std::string, std::string>> Wkiti::Database::query(std::string sql, const std::vector<std::string>& params) {
    if (type == DatabaseType::SQLITE) {
        std::vector<std::map<std::string, std::string>> results;
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "[DB 1.0 ERROR]: Query Prepare failed: " << sqlite3_errmsg(db) << std::endl;
            return results;
        }

        for (int i = 0; i < (int)params.size(); ++i) {
            sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<std::string, std::string> row;
            int count = sqlite3_column_count(stmt);
            for (int i = 0; i < count; i++) {
                const char* name = sqlite3_column_name(stmt, i);
                const unsigned char* text = sqlite3_column_text(stmt, i);
                row[name] = text ? reinterpret_cast<const char*>(text) : "NULL";
            }
            results.push_back(row);
        }

        sqlite3_finalize(stmt);
        return results;
    }
    return query(sql); // Fallback query execution
}