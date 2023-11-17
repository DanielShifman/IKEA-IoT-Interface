#ifndef SMARTHUB_DEVICES_H
#define SMARTHUB_DEVICES_H

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

class Devices {
public:
    std::map<std::string, std::string> devices;

    Devices(const std::string &path);
    [[maybe_unused]] Devices() = default;

    ~Devices() = default;

    void Load(const std::string& path);
    void Save();

    [[nodiscard]] std::string toString() const;;
};


#endif //SMARTHUB_DEVICES_H
