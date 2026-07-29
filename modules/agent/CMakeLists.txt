find_package(OpenSSL REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT TARGET OpenSSL::SSL)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENSSL REQUIRED openssl)

    add_library(OpenSSL::SSL INTERFACE IMPORTED)
    target_link_libraries(OpenSSL::SSL INTERFACE ${OPENSSL_LIBRARIES})
    target_include_directories(OpenSSL::SSL INTERFACE ${OPENSSL_INCLUDE_DIRS})

    add_library(OpenSSL::Crypto INTERFACE IMPORTED)
    target_link_libraries(OpenSSL::Crypto INTERFACE ${OPENSSL_LIBRARIES})
    target_include_directories(OpenSSL::Crypto INTERFACE ${OPENSSL_INCLUDE_DIRS})
endif()

find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

add_subdirectory(shell_controller)

add_executable(agent main.cc)

target_link_libraries(agent PRIVATE
    shell_controller
    agent_proto_lib  
    OpenSSL::SSL
    OpenSSL::Crypto
    gRPC::grpc++_reflection
    gRPC::grpc++
)

install(TARGETS agent DESTINATION bin)