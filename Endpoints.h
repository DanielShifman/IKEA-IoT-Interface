#ifndef SMARTHUB_ENDPOINTS_H
#define SMARTHUB_ENDPOINTS_H

#include <unordered_map>
#include <string>


enum class Endpoints {
    Status,
    Home,
    Devices,
    DeviceSet,
    Rooms,
    Scenes,
    Groups,
    ActivityLog,
    Users,
    FirmwareUpdate,
    Authorise,
    Token
};

const std::unordered_map<Endpoints, std::string> endpointStrings{
        {Endpoints::Status, "/v1/hub/status"},
        {Endpoints::Home, "/v1/home"},
        {Endpoints::Devices, "/v1/devices"},
        {Endpoints::DeviceSet, "/v1/device-set"},
        {Endpoints::Rooms, "/v1/rooms"},
        {Endpoints::Scenes, "/v1/scenes"},
        {Endpoints::Groups, "/v1/groups"},
        {Endpoints::ActivityLog, "/v1/activitylog"},
        {Endpoints::Users, "/v1/users"},
        {Endpoints::FirmwareUpdate, "/v1/hub/ota/check"},
        {Endpoints::Authorise, "/v1/oauth/authorize"},
        {Endpoints::Token, "/v1/oauth/token"}
};


#endif //SMARTHUB_ENDPOINTS_H
