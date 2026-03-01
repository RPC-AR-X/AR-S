//Standard includes
#include <iostream>
#include <memory>
#include <string>

//gRPC includes
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include "agent.grpc.pb.h"

#include "shell_controller.hh"

//TODO: Need to implement PQC securing of gRPC channel

int main(int argc, char** argv) {

    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    Ars::Controller::Shell::ProcessingImplemantation service;
    grpc::ServerBuilder builder;

    // Set the server address and port
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // Start the server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    if (!server) {
        std::cerr << "Failed to start server" << std::endl;
        return -1;
    } else {
        std::cout << "Server started on port 50051" << std::endl;
        server->Wait();
    }
    return 0;
}
