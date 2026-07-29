//Standard includes
#include <string>

#include "providers/github_provider.h"

GitHubProvider::GitHubProvider() {
    std::cout << "Clear Start. Wait for token..." << "\n";
}

std::string GitHubProvider::FetchStatusAsJson() const{
    std::string pipeline_info = "";
    if (m_current_token.empty()) {
        return "";
    }

    try {
        std::string auth_header = "token " + m_current_token;

        httplib::Client cli("https://api.github.com");
        cli.set_connection_timeout(30, 0);  // 30 seconds
        cli.set_read_timeout(60, 0);        // 60 seconds

        httplib::Headers headers = {
            {"User-Agent", "ARS-Sonar-Module-Test/1.0"}, {"Accept", "application/vnd.github.v3+json"}, {"Authorization", auth_header}};

        auto result = cli.Get("/repos/RPC-AR-X/AR-S/actions/runs", headers);

        if (result && result->status == 200) {
            std::cout << "Status Code: " << result->status << "\n";
            if (!result->body.empty()) {
                try {
                    nlohmann::json raw_json = nlohmann::json::parse(result->body);
                    nlohmann::json filtered_json = nlohmann::json::array();

                    if (raw_json.contains("workflow_runs")) {
                        for (auto& el : raw_json["workflow_runs"]) {
                            nlohmann::json item;
                            item["id"] = el["id"];
                            item["name"] = el["name"];
                            item["status"] = el["status"];
                            item["conclusion"] = el["conclusion"];

                            filtered_json.push_back(item);
                        }
                    }

                    std::ofstream jsonfile;
                    jsonfile.open("file");
                    jsonfile << filtered_json;
                    jsonfile.close();

                    pipeline_info = filtered_json.dump(4);

                    return pipeline_info;
                } catch (const nlohmann::json::exception& e) {
                    std::cerr << "JSON parsing error: " << e.what() << "\n";
                    return "";
                }
            }
            return pipeline_info;
        } else if (result) {
            std::cerr << "HTTP Error: Status " << result->status << "\n";
            if (!result->body.empty()) {
                std::cerr << "Response: " << result->body << "\n";
            }
            return "";
        } else {
            auto err = result.error();
            std::cerr << "HTTP request failed: " << httplib::to_string(err) << "\n";
            return "";
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in StartFetching: " << e.what() << "\n";
        return "";
    }
}

std::string GitHubProvider::GetProviderName() const {
    return "Github";
}

void GitHubProvider::SetToken(const std::string& token) {
    m_current_token = token;
    std::cout << "Token Updated" << "\n";
}
