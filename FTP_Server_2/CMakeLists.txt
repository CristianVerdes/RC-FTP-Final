cmake_minimum_required(VERSION 3.6)
project(FTP_Server_2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sqlite/)
set(sqlite3_srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/sqlite3.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/sqlite3.h)
add_library(mylib SHARED
        ${sqlite3_srcs}
        )

set(CMAKE_CXX_STANDARD 11)

include_directories(sqlite)

set(SOURCE_FILES main.cpp Connection.cpp Connection.h Commands.cpp Commands.h createDB.cpp createDB.h)
set(HEADER_FILES ${sqlite}/sqlite3.h)
add_executable(FTP_Server_2 ${SOURCE_FILES})
find_library(SQLITE3_LIBRARY sqlite3)
target_link_libraries(FTP_Server_2
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${SQLITE3_LIBRARY}
        ${THREAD_LIBRARY} -pthread
        ${CMAKE_THREAD_LIBS_INIT}
        )