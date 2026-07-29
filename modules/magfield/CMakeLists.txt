find_package(PkgConfig REQUIRED)
find_package(sdbus-c++ REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pkg_check_modules(LIBNL REQUIRED IMPORTED_TARGET libnl-3.0 libnl-genl-3.0)

add_executable(magfield main.cpp)

target_link_libraries(magfield PRIVATE 
    PkgConfig::LIBNL 
    SDBusCpp::sdbus-c++
)