#ifndef SMARTHUB_AUTHENTICATOR_H
#define SMARTHUB_AUTHENTICATOR_H

#define MAX_DEPTH 5

#include "Endpoints.h"
#include "external/httplib/httplib.h"
#include "external/cryptopp/sha.h"
#include "external/cryptopp/base64.h"
#include <nlohmann/json.hpp>

#ifdef _WIN32
// Windows-specific headers
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
// Linux-specific headers
#include <unistd.h>
#include <netinet/ip_icmp.h>
#endif

class Authenticator {
public:
    Authenticator() = default;

    ~Authenticator() = default;

    [[maybe_unused]] static std::string CreateChallenge(const std::string& verification);
    [[maybe_unused]] static std::string Authorise(const std::string& ip, const int& port, const std::string& verification);
    [[maybe_unused]] static std::string GetToken(const std::string& ip, const int& port, const std::string& authCode, const std::string& verification, const int& depth);
    [[maybe_unused]] static std::string GetVerification();
    static bool TestConnection(const std::string& ip, const int& timeout);

    [[maybe_unused]] static std::string Status(const std::string& ip, const int& port);
};


#endif //SMARTHUB_AUTHENTICATOR_H
