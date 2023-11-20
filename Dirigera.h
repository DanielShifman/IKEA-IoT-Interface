#ifndef SMARTHUB_DIRIGERA_H
#define SMARTHUB_DIRIGERA_H

#include "Endpoints.h"
#include <nlohmann/json.hpp>
#include "external/httplib/httplib.h"

class Dirigera {
private:
    std::string ip;
    std::string port;
    std::string token;

public:
    Dirigera(std::string ip, std::string port, std::string token) : ip(std::move(ip)), port(std::move(port)), token(std::move(token)) {}
    ~Dirigera() = default;

    nlohmann::ordered_json getDevices();
    nlohmann::ordered_json getDevice(const std::string& id);
    void identifyDevice(const std::string& id);

    template <typename T>
    void setDeviceAttribute(const std::string& id, const std::string& attribute, const T& value);
    void setDeviceAttributes(const std::string& id, const std::string& attributes);
};

#include "Dirigera.tpp"

#endif //SMARTHUB_DIRIGERA_H
