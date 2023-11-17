template <typename T>
void Dirigera::setDeviceAttribute(const std::string &id, const std::string &attribute, const T &value) {
    httplib::Client client("https://" + this->ip + ":" + this->port);
    // disable certificate verification
    client.set_ca_cert_path("");
    client.enable_server_certificate_verification(false);
    httplib::Headers headers = {
            {"Authorization", "Bearer " + this->token}
    };
    nlohmann::json attr;
    attr.push_back({
                           {"attributes", {
                                   {attribute, value}
                           }}
                   });
    auto res = client.Patch(endpointStrings.at(Endpoints::Devices) + "/" + id, headers, attr.dump(), "application/json");
    if (res && res->status == 202) {
        return;
    } else {
        throw std::runtime_error("Could not set device attribute.");
    }
}