# 🚀 Getting Started & Installation

## Prerequisites
- **Compiler:** `g++` or `clang++` with C++17 support.
- **Dependencies:** OpenSSL, Winsock2 (Windows) / pthread (Linux), SQLite3.

## Environment Setup (Windows MSYS2 UCRT64)

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-openssl

Creating Your First App
Use the wkiti-cli generator:

code Powershell

# 1. Compile the CLI tool
g++ -std=c++17 src/cli.cpp -o wkiti-cli.exe

# 2. Create a new app
./wkiti-cli.exe create MyApp

# 3. Navigating into Project Directory

e.g E:\Wkiti\cd MyApp
    E:\Wkiti\MyApp

Quick Building
To compile your application:
code Powershell

g++ -O3 -std=c++17 -I./include -I"C:/msys64/ucrt64/include" `
src/main.cpp src/Server.cpp src/Database.cpp src/Model.cpp src/Request.cpp src/Response.cpp src/Security.cpp src/sqlite3.o `
-o app.exe `
-L"C:/msys64/ucrt64/lib" -lws2_32 -lssl -lcrypto -lcrypt32

And then run it:
code Powershell

./app.exe


[DB]: Connected to wkiti_app.db
[SECURITY]: HTTPS initialized.
[SERVER]: https://localhost:8443 running with 4 worker threads.