#include "Wkiti.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono> 

namespace fs = std::filesystem;

#pragma comment(lib, "Ws2_32.lib")

// --- PHASE 18: ThreadPool Implementation ---
Wkiti::ThreadPool::ThreadPool(size_t threads) : stop(false) {
    for(size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] {
            for(;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
                    if(this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task(); 
            }
        });
    }
}

void Wkiti::ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

Wkiti::ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers) worker.join();
}

// --- Server Constructor / Destructor ---
Wkiti::Server::Server() : is_https(false), ssl_ctx(nullptr), thread_pool(nullptr) {}

Wkiti::Server::~Server() {
    if (ssl_ctx) SSL_CTX_free(ssl_ctx);
    if (thread_pool) delete thread_pool;
}

// --- PHASE 16: HTTPS Engine Setup ---
void Wkiti::Server::use_https(std::string cert_file, std::string key_file) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    const SSL_METHOD *method = TLS_server_method();
    ssl_ctx = SSL_CTX_new(method);

    if (!ssl_ctx) {
        std::cerr << "[SSL ERROR]: Context creation failed" << std::endl;
        return;
    }

    if (SSL_CTX_use_certificate_file(ssl_ctx, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ssl_ctx, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }
    is_https = true;
    std::cout << "[SECURITY]: HTTPS initialized." << std::endl;
}

// --- Route Registration (Phase 3 & 10) ---
void Wkiti::Server::get(std::string path, Handler handler) { routes["GET " + path] = handler; }
void Wkiti::Server::post(std::string path, Handler handler) { routes["POST " + path] = handler; }
void Wkiti::Server::put(std::string path, Handler handler) { routes["PUT " + path] = handler; }
void Wkiti::Server::del(std::string path, Handler handler) { routes["DELETE " + path] = handler; }

// --- Static Files & Middleware (Phase 4 & 6) ---
void Wkiti::Server::static_files(std::string path) { static_dir = path; }
void Wkiti::Server::use(Middleware m) { middlewares.push_back(m); }

// --- VERSION 1.0: Error Handling ---
void Wkiti::Server::on_error(int status_code, Handler handler) {
    error_handlers[status_code] = handler;
}

void Wkiti::Server::trigger_error(int status_code, const Request& req, Response& res) {
    res.status_code = status_code;
    
    // Check if the developer defined a custom error page in main.cpp
    if (error_handlers.count(status_code)) {
        error_handlers[status_code](req, res); 
    } else {
        // Fallback default error pages
        res.headers["Content-Type"] = "text/html";
        if (status_code == 404) res.body = "<h1>404 Not Found</h1><p>Wkiti Framework</p>";
        else if (status_code == 500) res.body = "<h1>500 Internal Server Error</h1><p>Wkiti Framework</p>";
        else res.body = "<h1>Error " + std::to_string(status_code) + "</h1>";
    }
}

// --- Phase 1, 13 & 18: Optimized Listener ---
void Wkiti::Server::listen(int port, int thread_count) {
    thread_pool = new ThreadPool(thread_count);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    ::listen(server_fd, SOMAXCONN);

    std::string protocol = is_https ? "https" : "http";
    std::cout << "[SERVER]: " << protocol << "://localhost:" << port << " running with " 
              << thread_count << " worker threads." << std::endl;

    while (true) {
        SOCKET client_socket = accept(server_fd, NULL, NULL);
        if (client_socket == INVALID_SOCKET) continue;

        thread_pool->enqueue([this, client_socket] {
            this->handle_client((unsigned long long)client_socket);
        });
    }
}

// --- PHASE 13, 16, 18 & 1.0: The Worker Logic ---
void Wkiti::Server::handle_client(unsigned long long client_handle) {
    auto start_time = std::chrono::high_resolution_clock::now();

    SOCKET client_socket = (SOCKET)client_handle;
    SSL* ssl = nullptr;
    char buffer[30000] = {0};
    int bytes_received = 0;

    // SSL Handshake
    if (is_https) {
        ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, (int)client_socket);
        if (SSL_accept(ssl) <= 0) {
            SSL_free(ssl);
            closesocket(client_socket);
            return; 
        }
        bytes_received = SSL_read(ssl, buffer, 30000);
    } else {
        bytes_received = recv(client_socket, buffer, 30000, 0);
    }

    if (bytes_received <= 0) {
        if (ssl) SSL_free(ssl);
        closesocket(client_socket);
        return; 
    }

    Request req = Request::parse(buffer);
    Response res;

    if (req.is_websocket()) { /* Handshake logic would go here */ }

    // Phase 6 Pipeline
    bool should_continue = true;
    for (auto& m : middlewares) {
        if (!m(req, res)) { should_continue = false; break; }
    }

    // --- VERSION 1.0: CRASH PROTECTION & ROUTING ---
    if (should_continue) {
        std::string route_key = req.method + " " + req.path;
        
        try {
            if (routes.count(route_key)) {
                routes[route_key](req, res);
            } else if (!static_dir.empty() && req.method == "GET") {
                std::string file_path = static_dir + req.path;
                if (fs::exists(file_path) && !fs::is_directory(file_path)) {
                    std::ifstream file(file_path, std::ios::binary);
                    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    res.body = content;

                    std::string ext = fs::path(file_path).extension().string();
                    if (ext == ".html") res.headers["Content-Type"] = "text/html";
                    else if (ext == ".css") res.headers["Content-Type"] = "text/css";
                    else if (ext == ".js") res.headers["Content-Type"] = "application/javascript";
                    else if (ext == ".png") res.headers["Content-Type"] = "image/png";
                } else {
                    trigger_error(404, req, res); // File not found
                }
            } else {
                trigger_error(404, req, res); // Route not found
            }
        } 
        // GRACEFUL FAILURE: Catch C++ Exceptions so the server doesn't die
        catch (const std::exception& e) {
            std::cerr << "[CRITICAL EXCEPTION]: " << e.what() << " on path " << req.path << std::endl;
            trigger_error(500, req, res);
        }
        catch (...) {
            std::cerr << "[CRITICAL EXCEPTION]: Unknown error on path " << req.path << std::endl;
            trigger_error(500, req, res);
        }
    }

    // Phase 16: Send Response
    std::string response_str = res.to_string();
    if (is_https) {
        SSL_write(ssl, response_str.c_str(), (int)response_str.size());
        SSL_shutdown(ssl);
        SSL_free(ssl);
    } else {
        send(client_socket, response_str.c_str(), (int)response_str.size(), 0);
    }

    closesocket(client_socket);

    // Benchmarking Timers
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    total_requests++;
    total_microseconds += duration;
}