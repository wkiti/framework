cmake_minimum_required(VERSION 3.10)
project(WkitiFramework)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(wkiti_app 
    src/main.cpp 
    src/Server.cpp 
    src/Request.cpp 
    src/Response.cpp
)