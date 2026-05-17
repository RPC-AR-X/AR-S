add_library(shell_controller
    src/shell_controller.cc
    src/fd_handler.cc
)

target_include_directories(shell_controller PUBLIC
    include
)

target_link_libraries(shell_controller PUBLIC
    agent_proto_lib      
    gRPC::grpc++
    protobuf::libprotobuf
)