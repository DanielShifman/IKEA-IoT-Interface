#ifndef SMARTHUB_DEVICES_H
#define SMARTHUB_DEVICES_H

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

class Devices {
private:
    std::map<std::string, std::string> devices;
    std::string devicePath;
public:

    explicit Devices(const std::string &path);
    [[maybe_unused]] Devices() = default;

    ~Devices() = default;

    void Load(const std::string& path);
    void Save();
    void Save(const std::string& path);
    [[nodiscard]] std::map<std::string, std::string> getDevices() const {return devices;};
    [[nodiscard]] std::string getDevice(const std::string& name) const {return devices.at(name);};
    void addDevice(const std::string& name, const std::string& id) {devices.insert(std::pair<std::string, std::string>(name, id));};
    [[nodiscard]] bool isEmpty() const {return devices.empty();};
    [[maybe_unused]] [[nodiscard]] std::string toString() const;;

    std::map<std::string, std::string>::iterator begin() {return devices.begin();};
    std::map<std::string, std::string>::iterator end() {return devices.end();};
};


#endif //SMARTHUB_DEVICES_H
