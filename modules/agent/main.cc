//Standard includes
#include <signal.h>
#include <string.h>

#include <memory>
#include <string>
#include <stdexcept>

//Libs
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include "agent.grpc.pb.h"

#include <spdlog/spdlog.h>

#include "proc_impl.hh"

//TODO: Need to implement PQC securing of gRPC channel

int main(int argc, char** argv) {

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDWAIT;

    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        throw std::runtime_error("Failed to set SIGCHLD action: " + std::string(strerror(errno)));
    }

    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    Ars::Controller::Shell::ProcessingImplementation service;
    grpc::ServerBuilder builder;

    // Set the server address and port
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // Start the server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    if (!server) {
        spdlog::error("Failed to start server");
        return -1;
    } else {
        spdlog::info("Server started on port 50051");
        server->Wait();
    }
    return 0;
}
