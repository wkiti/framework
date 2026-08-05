#ifndef WKITI_HPP
#define WKITI_HPP

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <thread>
#include <queue>              
#include <mutex>              
#include <condition_variable> 
#include <atomic>             

// External Libraries
#include "sqlite3.h"           
#include "nlohmann/json.hpp"   
#include <openssl/ssl.h>       
#include <openssl/err.h>       

// Optional Native Database C Drivers
#if __has_include(<postgresql/libpq-fe.h>)
#include <postgresql/libpq-fe.h>
#define WKITI_HAS_POSTGRES
#endif

#if __has_include(<mysql/mysql.h>)
#include <mysql/mysql.h>
#define WKITI_HAS_MYSQL
#endif

namespace Wkiti {

    enum class DatabaseType {
        SQLITE,
        POSTGRESQL,
        MYSQL
    };

    class ThreadPool {
    public:
        ThreadPool(size_t threads);
        void enqueue(std::function<void()> task);
        ~ThreadPool();
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
    };

    struct Request {
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;

        static Request parse(std::string raw_data);

        bool is_websocket() const {
            auto it = headers.find("Upgrade");
            return it != headers.end() && it->second == "websocket";
        }
    };

    struct Response {
        int status_code = 200;
        std::string body;
        std::map<std::string, std::string> headers;
        std::vector<std::string> cookies; 

        static std::map<std::string, std::string> template_cache;
        static std::mutex cache_mutex;

        std::string to_string();
        void render(std::string template_name, std::map<std::string, std::string> context); 
        void set_cookie(std::string name, std::string value); 
        void json(nlohmann::json data); 
    };

    using Handler = std::function<void(const Request&, Response&)>;
    using Middleware = std::function<bool(const Request&, Response&)>;

    class Security {
    public:
        static std::string escape_sql(std::string input);
        static std::string escape_html(std::string input);
        static std::string hash_password(std::string password);
    };

    // MULTI-DATABASE ENGINE (SQLite3, PostgreSQL libpq, MySQL libmariadb)
    class Database {
    public:
        Database(std::string conn_str);
        ~Database();

        DatabaseType get_type() const { return type; }
        
        bool execute(std::string sql);
        std::vector<std::map<std::string, std::string>> query(std::string sql);

        bool execute(std::string sql, const std::vector<std::string>& params);
        std::vector<std::map<std::string, std::string>> query(std::string sql, const std::vector<std::string>& params);

    private:
        DatabaseType type;
        std::string connection_string;
        sqlite3* db = nullptr;

#ifdef WKITI_HAS_POSTGRES
        PGconn* pg_conn = nullptr;
#endif

#ifdef WKITI_HAS_MYSQL
        MYSQL* mysql_conn = nullptr;
#endif

        void init_sqlite(const std::string& db_name);
        void init_postgresql(const std::string& conn_uri);
        void init_mysql(const std::string& conn_uri);
        std::string normalize_sql_for_postgres(const std::string& sql);
    };

    class Model {
    protected:
        static Database* db_ptr;
    public:
        static void setDatabase(Database* db);
        int id = 0;
        virtual std::map<std::string, std::string> toMap() = 0;
        virtual void fromMap(std::map<std::string, std::string> data) = 0;
    };

    class User : public Model {
    public:
        std::string name;
        std::string email;
        std::string password;

        static std::vector<User> all();
        static User find(int id);
        static User find_by_email(std::string email);
        void save();

        std::map<std::string, std::string> toMap() override;
        void fromMap(std::map<std::string, std::string> data) override;
    };

    class Server {
    public:
        Server();
        ~Server();

        void use_https(std::string cert_file, std::string key_file);
        void get(std::string path, Handler handler);
        void post(std::string path, Handler handler);
        void put(std::string path, Handler handler);
        void del(std::string path, Handler handler);
        void static_files(std::string path); 
        void use(Middleware m);             
        void on_error(int status_code, Handler handler);
        void listen(int port, int thread_count = 4);

        long long get_total_requests() const { return total_requests.load(); }
        long long get_total_microseconds() const { return total_microseconds.load(); }

    private:
        void handle_client(unsigned long long client_socket);
        void trigger_error(int status_code, const Request& req, Response& res);

        bool is_https;
        SSL_CTX* ssl_ctx;
        ThreadPool* thread_pool;

        std::map<std::string, Handler> routes;
        std::map<int, Handler> error_handlers; 
        std::string static_dir;
        std::vector<Middleware> middlewares;

        std::atomic<long long> total_requests{0};
        std::atomic<long long> total_microseconds{0};
    };
}

#endif