template <typename T>
void Dirigera::setDeviceAttribute(const std::string &id, const std::string &attribute, const T &attributeValue, bool check) {
    httplib::Client client("https://" + this->ip + ":" + this->port);
    // disable certificate verification
    client.set_ca_cert_path("");
    client.enable_server_certificate_verification(false);
    httplib::Headers headers = {
            {"Authorization", "Bearer " + this->token}
    };

    nlohmann::basic_json<> attrObj = {
            {"attributes", {
                    {attribute, attributeValue}
            }}
    };
    if (check) {
        // If T is string, and that string is "auto", then get current state of device and set attribute to the opposite of that.
        if (std::is_same<T, std::string>::value) {
            if (reinterpret_cast<const std::string &>(attributeValue) == "auto") {
                auto device = this->getDevice(id);
                bool val = device["attributes"][attribute];
                attrObj = {
                        {"attributes", {
                                {attribute, !val}
                        }}
                };
            } else {
                throw std::runtime_error("Invalid value for attribute.");
            }
        }
    }
    nlohmann::json attr;
    attr.push_back(attrObj);
    auto res = client.Patch(endpointStrings.at(Endpoints::Devices) + "/" + id, headers, attr.dump(), "application/json");
    if (res && res->status == 202) {
        return;
    } else {
        throw std::runtime_error("Could not set device attribute.");
    }
}