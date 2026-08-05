#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void create_directory(const std::string& path) {
    if (!fs::exists(path)) {
        fs::create_directory(path);
        std::cout << "  \033[32m[✔] Created Folder:\033[0m " << path << std::endl;
    }
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    file << content;
    file.close();
    std::cout << "  \033[32m[✔] Created File:  \033[0m " << path << std::endl;
}

void copy_file_safe(const std::string& src, const std::string& dest) {
    if (fs::exists(src)) {
        fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
        std::cout << "  \033[36m[➜] Copied Asset:  \033[0m " << src << " -> " << dest << std::endl;
    } else {
        std::cout << "  \033[33m[!] Warning: Could not find \033[0m" << src << " to copy." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "\n\033[1;36m====================================================\033[0m" << std::endl;
        std::cout << "\033[1;32m  WKITI C++ WEB FRAMEWORK CLI v1.0\033[0m" << std::endl;
        std::cout << "\033[1;36m====================================================\033[0m\n" << std::endl;
        std::cout << "  Usage: \033[1;33mwkiti-cli create <project_name>\033[0m\n" << std::endl;
        return 1;
    }

    std::string command = argv[1];
    std::string projectName = argv[2];

    if (command == "create") {
        std::cout << "\n\033[1;36m====================================================\033[0m" << std::endl;
        std::cout << "\033[1;32m  WKITI C++ WEB FRAMEWORK - SCAFFOLDING ENGINE\033[0m" << std::endl;
        std::cout << "\033[1;36m====================================================\033[0m\n" << std::endl;
        std::cout << "🚀 Generating High-Level Professional C++ App: \033[1;33m" << projectName << "\033[0m...\n" << std::endl;

        // 1. Create Folder Tree
        create_directory(projectName);
        create_directory(projectName + "/src");
        create_directory(projectName + "/include");
        create_directory(projectName + "/include/nlohmann");
        create_directory(projectName + "/templates");
        create_directory(projectName + "/public");

        // 2. Copy Wkiti Framework Dependencies
        copy_file_safe("include/Wkiti.hpp", projectName + "/include/Wkiti.hpp");
        copy_file_safe("include/sqlite3.h", projectName + "/include/sqlite3.h");
        copy_file_safe("include/nlohmann/json.hpp", projectName + "/include/nlohmann/json.hpp");

        copy_file_safe("src/Server.cpp", projectName + "/src/Server.cpp");
        copy_file_safe("src/Database.cpp", projectName + "/src/Database.cpp");
        copy_file_safe("src/Model.cpp", projectName + "/src/Model.cpp");
        copy_file_safe("src/Request.cpp", projectName + "/src/Request.cpp");
        copy_file_safe("src/Response.cpp", projectName + "/src/Response.cpp");
        copy_file_safe("src/Security.cpp", projectName + "/src/Security.cpp");
        copy_file_safe("src/sqlite3.o", projectName + "/src/sqlite3.o");

        // 3. Generate High-Level main.cpp
        std::string main_cpp = R"cpp(#include "Wkiti.hpp"
#include <iostream>
#include <atomic>

using json = nlohmann::json;

// Thread-safe request counter
std::atomic<long long> global_click_count{0};

int main() {
    Wkiti::Server app;
    Wkiti::Database db("app.db");
    Wkiti::Model::setDatabase(&db);

    // Serve static frontend assets (/public/style.css, /public/script.js)
    app.static_files("public");

    // Home Page - Renders templates/index.html
    app.get("/", [](const Wkiti::Request& req, Wkiti::Response& res) {
        res.render("index.html", {
            {"title", "My Wkiti C++ App"},
            {"status", "Online & High-Performance"},
            {"engine", "Wkiti C++ v1.0 Core"}
        });
    });

    // REST API Endpoint 1: Server Metrics
    app.get("/api/stats", [&app](const Wkiti::Request& req, Wkiti::Response& res) {
        res.json({
            {"framework", "Wkiti C++"},
            {"server_status", "Active"},
            {"clicks", global_click_count.load()},
            {"total_requests", app.get_total_requests()}
        });
    });

    // REST API Endpoint 2: Increment Counter
    app.post("/api/increment", [](const Wkiti::Request& req, Wkiti::Response& res) {
        global_click_count++;
        res.json({
            {"status", "success"},
            {"new_count", global_click_count.load()}
        });
    });

    std::cout << "\n\033[1;32m[WKITI SERVER]: Application running at http://localhost:8080\033[0m\n" << std::endl;
    app.listen(8080, 4);
    return 0;
}
)cpp";

        write_file(projectName + "/src/main.cpp", main_cpp);

        // 4. Generate Standard HTML5 Template (templates/index.html)
        std::string index_html = R"html(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{{title}}</title>
    <link rel="stylesheet" href="/style.css">
    <script src="/script.js" defer></script>
</head>
<body>
    <div class="app-container">
        <!-- HEADER NAVBAR -->
        <header class="navbar">
            <div class="logo-brand">
                <div class="logo-badge">&gt;_</div>
                <span class="logo-text">WKITI</span>
            </div>
            <div class="status-pill">
                <span class="pulse-dot"></span>
                <span>{{status}}</span>
            </div>
        </header>

        <!-- HERO SECTION -->
        <main class="hero-section">
            <div class="hero-badge">⚡ Powered by Native C++ Engine</div>
            <h1 class="hero-title">High Performance Web Development in <span class="gradient-text">C++</span></h1>
            <p class="hero-subtitle">Your standalone enterprise application created with Wkiti Framework. Fast, secure, and production-ready.</p>

            <!-- INTERACTIVE C++ API CARD -->
            <div class="interactive-card">
                <div class="card-header">
                    <h3>Interactive C++ Backend Counter</h3>
                    <span class="api-tag">REST API Active</span>
                </div>
                <p>Click below to send an asynchronous POST request to your compiled C++ backend:</p>
                
                <div class="counter-box">
                    <span class="counter-label">Server Counter:</span>
                    <span id="click-counter" class="counter-value">0</span>
                </div>

                <div class="button-group">
                    <button id="btn-increment" class="btn btn-primary">Increment Counter +1</button>
                    <button id="btn-refresh" class="btn btn-secondary">Refresh Stats</button>
                </div>
            </div>

            <!-- FEATURE GRID -->
            <div class="feature-grid">
                <div class="feature-card">
                    <div class="icon">🚀</div>
                    <h4>Sub-Millisecond Speed</h4>
                    <p>Zero interpreter overhead with compiled native binary execution.</p>
                </div>
                <div class="feature-card">
                    <div class="icon">🔒</div>
                    <h4>OpenSSL Security</h4>
                    <p>Built-in SHA-256 password hashing and TLS 1.3 encryption.</p>
                </div>
                <div class="feature-card">
                    <div class="icon">🗄️</div>
                    <h4>SQLite ORM</h4>
                    <p>Type-safe prepared statements protecting against SQL injection.</p>
                </div>
            </div>
        </main>

        <footer class="footer">
            <p>© 2026 {{title}} — Built with Wkiti C++ Framework</p>
        </footer>
    </div>
</body>
</html>
)html";

        write_file(projectName + "/templates/index.html", index_html);

        // 5. Generate Professional CSS Stylesheet (public/style.css)
        std::string style_css = R"css(:root {
    --bg-color: #fafafa;
    --card-bg: #ffffff;
    --border-color: #e5e7eb;
    --text-primary: #111827;
    --text-secondary: #6b7280;
    --accent-blue: #0070f3;
    --accent-emerald: #10b981;
}

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
}

body {
    background-color: var(--bg-color);
    color: var(--text-primary);
    line-height: 1.6;
}

.app-container {
    max-width: 1100px;
    margin: 0 auto;
    padding: 0 24px;
}

/* NAVBAR */
.navbar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 24px 0;
    border-bottom: 1px solid var(--border-color);
}

.logo-brand {
    display: flex;
    align-items: center;
    gap: 12px;
}

.logo-badge {
    background: #000000;
    color: #00F0FF;
    width: 38px;
    height: 38px;
    border-radius: 10px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-family: monospace;
    font-weight: 900;
    font-size: 18px;
}

.logo-text {
    font-weight: 900;
    font-size: 20px;
    letter-spacing: 2px;
}

.status-pill {
    display: flex;
    align-items: center;
    gap: 8px;
    background: #ecfdf5;
    color: #047857;
    border: 1px solid #a7f3d0;
    padding: 6px 16px;
    border-radius: 100px;
    font-size: 12px;
    font-weight: 600;
}

.pulse-dot {
    width: 8px;
    height: 8px;
    background-color: #10b981;
    border-radius: 50%;
    box-shadow: 0 0 8px #10b981;
}

/* HERO SECTION */
.hero-section {
    padding: 60px 0;
    text-align: center;
}

.hero-badge {
    display: inline-block;
    background: #eff6ff;
    color: #1d4ed8;
    border: 1px solid #bfdbfe;
    padding: 6px 16px;
    border-radius: 100px;
    font-size: 12px;
    font-weight: 700;
    margin-bottom: 20px;
}

.hero-title {
    font-size: 48px;
    font-weight: 900;
    line-height: 1.15;
    margin-bottom: 16px;
}

.gradient-text {
    background: linear-gradient(135deg, #0070f3 0%, #00dfd8 100%);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
}

.hero-subtitle {
    color: var(--text-secondary);
    font-size: 18px;
    max-width: 680px;
    margin: 0 auto 40px auto;
}

/* INTERACTIVE CARD */
.interactive-card {
    background: var(--card-bg);
    border: 1px solid var(--border-color);
    border-radius: 20px;
    padding: 36px;
    text-align: left;
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.05);
    margin-bottom: 50px;
}

.card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 12px;
}

.interactive-card h3 {
    font-size: 20px;
    font-weight: 800;
}

.api-tag {
    background: #f3f4f6;
    color: #374151;
    padding: 4px 10px;
    border-radius: 6px;
    font-size: 11px;
    font-family: monospace;
    font-weight: 700;
}

.counter-box {
    background: #f9fafb;
    border: 1px solid var(--border-color);
    padding: 20px;
    border-radius: 12px;
    margin: 20px 0;
    display: flex;
    align-items: center;
    justify-content: space-between;
}

.counter-label {
    font-weight: 700;
    font-size: 15px;
}

.counter-value {
    font-size: 32px;
    font-weight: 900;
    color: var(--accent-blue);
    font-family: monospace;
}

/* BUTTONS */
.button-group {
    display: flex;
    gap: 12px;
}

.btn {
    padding: 12px 24px;
    border-radius: 10px;
    font-weight: 700;
    font-size: 14px;
    cursor: pointer;
    transition: all 0.2s;
    border: none;
}

.btn-primary {
    background: #000000;
    color: #ffffff;
}

.btn-primary:hover {
    background: #222222;
    transform: translateY(-2px);
}

.btn-secondary {
    background: #ffffff;
    color: #111827;
    border: 1px solid var(--border-color);
}

.btn-secondary:hover {
    background: #f3f4f6;
}

/* FEATURE GRID */
.feature-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
    gap: 24px;
    text-align: left;
}

.feature-card {
    background: var(--card-bg);
    border: 1px solid var(--border-color);
    padding: 28px;
    border-radius: 16px;
}

.feature-card .icon {
    font-size: 28px;
    margin-bottom: 12px;
}

.feature-card h4 {
    font-size: 16px;
    font-weight: 800;
    margin-bottom: 8px;
}

.feature-card p {
    color: var(--text-secondary);
    font-size: 13px;
}

/* FOOTER */
.footer {
    padding: 40px 0;
    border-top: 1px solid var(--border-color);
    color: var(--text-secondary);
    font-size: 13px;
}
)css";

        write_file(projectName + "/public/style.css", style_css);

        // 6. Generate Interactive JavaScript (public/script.js)
        std::string script_js = R"js(// Wkiti Frontend Interactive JavaScript
document.addEventListener('DOMContentLoaded', () => {
    console.log('[WKITI JS]: Interactive Application Loaded.');

    const counterEl = document.getElementById('click-counter');
    const btnIncrement = document.getElementById('btn-increment');
    const btnRefresh = document.getElementById('btn-refresh');

    function fetchStats() {
        fetch('/api/stats')
            .then(res => res.json())
            .then(data => {
                if (counterEl) counterEl.textContent = data.clicks;
            })
            .catch(err => console.error('[API Error]:', err));
    }

    if (btnIncrement) {
        btnIncrement.addEventListener('click', () => {
            fetch('/api/increment', { method: 'POST' })
                .then(res => res.json())
                .then(data => {
                    if (counterEl) counterEl.textContent = data.new_count;
                })
                .catch(err => console.error('[API Error]:', err));
        });
    }

    if (btnRefresh) {
        btnRefresh.addEventListener('click', fetchStats);
    }

    fetchStats();
});
)js";

        write_file(projectName + "/public/script.js", script_js);

        std::cout << "\n\033[1;32m====================================================\033[0m" << std::endl;
        std::cout << "\033[1;32m  ✔ SUCCESS! Standalone Wkiti Project Created: ./" << projectName << "\033[0m" << std::endl;
        std::cout << "\033[1;32m====================================================\033[0m\n" << std::endl;

    } else {
        std::cout << "Unknown command: " << command << std::endl;
    }

    return 0;
}