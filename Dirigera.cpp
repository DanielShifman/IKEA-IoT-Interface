#include "Dirigera.h"

nlohmann::ordered_json Dirigera::getDevices() {
    httplib::Client client("https://" + this->ip + ":" + this->port);
    // disable certificate verification
    client.set_ca_cert_path("");
    client.enable_server_certificate_verification(false);
    httplib::Headers headers = {
            {"Authorization", "Bearer " + this->token}
    };
    auto res = client.Get(endpointStrings.at(Endpoints::Devices), headers);
    if (res && res->status == 200) {
        return nlohmann::ordered_json::parse(res->body);
    } else {
        throw std::runtime_error("Could not get devices.");
    }
}

nlohmann::ordered_json Dirigera::getDevice(const std::string &id) {
    httplib::Client client("https://" + this->ip + ":" + this->port);
    // disable certificate verification
    client.set_ca_cert_path("");
    client.enable_server_certificate_verification(false);
    httplib::Headers headers = {
            {"Authorization", "Bearer " + this->token}
    };
    auto res = client.Get(endpointStrings.at(Endpoints::Devices) + "/" + id, headers);
    if (res && res->status == 200) {
        return nlohmann::ordered_json::parse(res->body);
    } else {
        throw std::runtime_error("Could not get device.");
    }
}

void Dirigera::identifyDevice(const std::string &id) {
    httplib::Client client("https://" + this->ip + ":" + this->port);
    // disable certificate verification
    client.set_ca_cert_path("");
    client.enable_server_certificate_verification(false);
    httplib::Headers headers = {
            {"Authorization", "Bearer " + this->token}
    };
    auto res = client.Put(endpointStrings.at(Endpoints::Devices) + "/" + id + "/identify", headers, "", "application/json");
    if (res && res->status == 202) {
        return;
    } else {
        throw std::runtime_error("Could not identify device.");
    }
}

void Dirigera::setDeviceAttributes(const std::string &id, const nlohmann::ordered_json &attributes) {
    httplib::Client client("https://" + this->ip + ":" + this->port);
    // disable certificate verification
    client.set_ca_cert_path("");
    client.enable_server_certificate_verification(false);
    httplib::Headers headers = {
            {"Authorization", "Bearer " + this->token}
    };
    auto res = client.Patch(endpointStrings.at(Endpoints::Devices) + "/" + id, headers, attributes.dump(), "application/json");
    if (res && res->status == 202) {
        return;
    } else {
        throw std::runtime_error("Could not set device attribute.");
    }
}
