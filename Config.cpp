#include <iostream>
#include "Config.h"

using json = nlohmann::ordered_json;

Config::Config(const std::string &path) {

    Load(path);
};

void Config::Load(const std::string& path) {
    try {
        std::ifstream file(path, std::ios::in);
        // Error handling
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file.");
        }
        json jsonObj = json::parse(file);
        if (jsonObj.find("dirigera_ip") != jsonObj.end()) {
            ip = jsonObj["dirigera_ip"];
        } else {
            throw std::runtime_error("Missing 'ip' field in the JSON file.");
        }
        if (jsonObj.find("dirigera_port") != jsonObj.end()) {
            port = jsonObj["dirigera_port"];
        } else {
            throw std::runtime_error("Missing 'port' field in the JSON file.");
        }
        if (jsonObj.find("auth_code") != jsonObj.end()) {
            authCode = jsonObj["auth_code"];
        } else {
            std::cout << "No auth code found." << std::endl;
        }
        if (jsonObj.find("verification") != jsonObj.end()) {
            verification = jsonObj["verification"];
        } else {
            std::cout << "No verification found." << std::endl;
        }
        if (jsonObj.find("token") != jsonObj.end()) {
            token = jsonObj["token"];
        } else {
            std::cout << "No token found." << std::endl;
        }
    } catch (std::exception &e) {
        throw std::runtime_error(std::string(e.what()));
    }
};

void Config::Save() {
try {
        json jsonStr;
        if (!ip.empty()) {
            jsonStr["dirigera_ip"] = ip;
        }
        if (!port.empty()) {
            jsonStr["dirigera_port"] = port;
        }
        if (!authCode.empty()) {
            jsonStr["auth_code"] = authCode;
        }
        if (!verification.empty()) {
            jsonStr["verification"] = verification;
        }
        if (!token.empty()) {
            jsonStr["token"] = token;
        }
        std::ofstream file("../config.json", std::ios::out);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file.");
        }
        file << jsonStr.dump(4);
        file.close();
    } catch (std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
};
