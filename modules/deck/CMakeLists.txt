set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick DBus NetworkAuth)

add_executable(deck
        main.cpp
        lib/json_network_manager.hpp
        lib/oauth2_device_flow.hpp
        lib/oauth2_device_flow.cpp
        sonar/sonar_view_model.hpp
        sonar/sonar_view_model.cpp
        sonar/auth_manager.hpp
        sonar/auth_manager.cpp
        qml/qml.qrc
)

target_include_directories(deck PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(deck PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::DBus
    Qt6::NetworkAuth
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(
            CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
            -Werror -g -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address"
    )
    target_link_options(deck
            BEFORE PUBLIC -fsanitize=undefined PUBLIC -fsanitize=address
    )
endif()

install(TARGETS deck
    DESTINATION bin
)