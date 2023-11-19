#ifndef SMARTHUB_CONFIG_H
#define SMARTHUB_CONFIG_H

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

class Config {
public:
    std::string ip;
    std::string port;
    std::string authCode;
    std::string verification;
    std::string token;
    std::string configPath;

    [[maybe_unused]] Config(const std::string &_ip, const std::string &_port, const std::string &_authCode, const std::string &_verification, const std::string &_token):
            ip(_ip), port(_port), authCode(_authCode), verification(_verification), token(_token) {};

    [[maybe_unused]] Config() = default;

    ~Config() = default;

    explicit Config(const std::string &path);

    void Load(const std::string& path);

    [[nodiscard]] std::string toString() const {return "ip: " + ip + "\nport: " + port + "\nauthCode: " + authCode + "\nverification: " + verification + "\ntoken: " + token;};

    void Save();
};


#endif //SMARTHUB_CONFIG_H
