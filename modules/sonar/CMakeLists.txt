find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(sdbus-c++ REQUIRED)
find_package(sdbus-c++-tools REQUIRED)

FetchContent_Declare(httplib SYSTEM
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
        GIT_TAG a609330e4c6374f741d3b369269f7848255e1954
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(httplib)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

set(DBUS_INTERFACE_XML "${CMAKE_SOURCE_DIR}/protocols/dbus/sonar/fetchPipelineData.xml")
set(DBUS_ADAPTOR_H "${CMAKE_CURRENT_BINARY_DIR}/generated/PipelineAdaptor.h")
set(DBUS_PROXY_H "${CMAKE_CURRENT_BINARY_DIR}/generated/PipelineProxy.h")

file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
        OUTPUT ${DBUS_ADAPTOR_H} ${DBUS_PROXY_H}
        COMMAND SDBusCpp::sdbus-c++-xml2cpp ${DBUS_INTERFACE_XML} --adaptor=${DBUS_ADAPTOR_H} --proxy=${DBUS_PROXY_H}
        DEPENDS ${DBUS_INTERFACE_XML}
        COMMENT "Generating D-Bus C++ interfaces from ${DBUS_INTERFACE_XML}"
)

add_executable(sonar main.cpp
        src/adapters/dbus_adapter.cpp
        src/providers/github_provider.cpp
)
add_custom_target(generate_dbus_interfaces DEPENDS ${DBUS_ADAPTOR_H} ${DBUS_PROXY_H})

add_dependencies(sonar generate_dbus_interfaces)

target_include_directories(sonar PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/generated
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${libenvpp_SOURCE_DIR}/include
)

target_link_libraries(sonar PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        ZLIB::ZLIB
        httplib
        nlohmann_json::nlohmann_json
        SDBusCpp::sdbus-c++
        ${SDBUSCPP_GENERATED_INTERFACES_LIB}
)

target_compile_definitions(sonar PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(
            CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
            -Werror -g -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address"
    )
    target_link_options(sonar
            BEFORE PUBLIC -fsanitize=undefined PUBLIC -fsanitize=address
    )
endif()

install(TARGETS sonar DESTINATION bin)
install(
        FILES "${dbus-api_SOURCE_DIR}/sonar/fetchPipelineData.xml"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces
        RENAME org.ars.sonar.Interface.xml
)