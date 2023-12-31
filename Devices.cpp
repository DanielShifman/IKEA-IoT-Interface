#include <iostream>
#include "Devices.h"

using json = nlohmann::ordered_json;

Devices::Devices(const std::string &path) {
    this->devicePath = path;
    Load(path);
}

void Devices::Load(const std::string &path) {
    try {
        std::ifstream file(path, std::ios::in);
        // Error handling
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file.");
        }
        json jsonObj = json::parse(file);
        if (jsonObj.empty()) {
            throw std::runtime_error("Empty JSON file.");
        }
        for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
            devices[it.key()] = it.value();
        }
        file.close();
        this->devicePath = path;
    } catch (std::exception &e) {
        throw std::runtime_error(std::string(e.what()));
    }
}

[[maybe_unused]] std::string Devices::toString() const {
    std::string str;
    for (const auto & device : devices) {
        str += device.first + ": " + device.second + "\n";
    }
    return str;
}

void Devices::Save() {
    try {
        json jsonStr;
        if (!devices.empty()) {
            for (const auto & device : devices) {
                jsonStr[device.first] = device.second;
            }
        }
        std::ofstream file(this->devicePath, std::ios::out);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file.");
        }
        file << jsonStr.dump(4);
        file.close();
    } catch (std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void Devices::Save(const std::string &path) {
    this->devicePath = path;
    Save();
};