#include "Authenticator.h"

std::string Authenticator::Authorise(const std::string &ip, const int &port, const std::string &verification) {
    httplib::Client client("https://" + ip + ":" + std::to_string(port));
    // disable certificate verification
    client.set_ca_cert_path("");
    client.enable_server_certificate_verification(false);
    httplib::Headers headers = {
            {"Content-Type", "application/x-www-form-urlencoded"}
    };
    httplib::Params params = {
            {"response_type", "code"},
            {"code_challenge_method", "S256"},
            {"code_challenge", CreateChallenge(verification)}
    };
    auto res = client.Get(endpointStrings.at(Endpoints::Authorise), params, headers);
    if (res && res->status == 200) {
        // Extract code from json body
        nlohmann::json json = nlohmann::json::parse(res->body);
        return json["code"];
    } else {
        std::cout << "Error: " << res.error() << std::endl;
        std::cout << "Error: " << res->status << " - " << res->reason << std::endl;
        // Handle the error appropriately
        return "";
    }
}

std::string getHostname() {
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    return hostname;
}

std::string Authenticator::GetToken(const std::string &ip, const int &port, const std::string &authCode,
                                    const std::string &verification, const int &depth = 1) {
    httplib::Client client("https://" + ip + ":" + std::to_string(port));
    // disable certificate verification
    client.set_ca_cert_path("");
    client.enable_server_certificate_verification(false);
    httplib::Headers headers = {
            {"Content-Type", "application/x-www-form-urlencoded"}
    };
    std::string payload = "grant_type=authorization_code&code=" + authCode + "&name=" + getHostname() + "&code_verifier=" + verification;
    std::cout << payload << std::endl;
    std::cout << "Please press the action button on the hub then press enter to continue..." << std::endl;
    std::cin.ignore();
    auto res = client.Post(endpointStrings.at(Endpoints::Token), payload, "application/x-www-form-urlencoded");
    if (res && res->status == 200) {
        try {
            // Extract token from json body
            nlohmann::json json = nlohmann::json::parse(res->body);
            return json["access_token"];
        } catch (std::exception &e) {
            std::cout << "Error: " << e.what() << std::endl;
            throw e;
        }
    } else if (res && res->status == 401) {
        // json decode error
        nlohmann::json json = nlohmann::json::parse(res->body);
        if (json["error"] == "Invalid authorization code") {
            // reauthorise
            std::cout << "Invalid authorisation code. Reauthorising..." << std::endl;
            std::string newAuthCode = Authorise(ip, port, verification);
            if (newAuthCode.empty()) {
                throw std::runtime_error("Could not authorise.");
            }
            std::cout << "New authorisation code: " << newAuthCode << std::endl;
            if (depth > MAX_DEPTH) {
                throw std::runtime_error("Unable to authorise within maximum attempts ("+std::to_string(MAX_DEPTH)+").");
            } else
            return GetToken(ip, port, newAuthCode, verification, depth + 1);
        } else {
            std::cout << "Error: " << res->status << " - " << res->reason << std::endl;
            throw std::runtime_error(to_string(res.error()));
        }
    } else if (res && res->status == 403) {
        // json decode error
        nlohmann::json json = nlohmann::json::parse(res->body);
        // if error exists
        if (json.find("message") != json.end()) {
            throw std::runtime_error(std::string(json["message"]));
        }
    } else {
        std::cout << "Error: " << res.error() << std::endl;
        std::cout << "Error: " << res->status << " - " << res->reason << std::endl;
        // Handle the error appropriately
        throw std::runtime_error(to_string(res.error()));
    }

}

std::string Authenticator::Status(const std::string &ip, const int &port) {
    httplib::Client client("https://" + ip + ":" + std::to_string(port));
    // disable certificate verification
    client.set_ca_cert_path("");
    client.enable_server_certificate_verification(false);
    httplib::Headers headers = {
            {"Content-Type", "application/x-www-form-urlencoded"}
    };
    auto res = client.Get(endpointStrings.at(Endpoints::Status), headers);
    if (res && res->status == 200) {
        return res->body;
    } else {
        std::cout << "Error: " << res.error() << std::endl;
        std::cout << "Error: " << res->status << " - " << res->reason << std::endl;
        // Handle the error appropriately
        return "";
    }
}

std::string Authenticator::GetVerification() {
    const std::string AB = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
    std::string code;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dis(0, AB.size() - 1);

    for (int i = 0; i < 128; i++) {
        code += AB[dis(gen)];
    }

    return code;
}
#ifdef _WIN32

// Windows-specific code
std::string Authenticator::CreateChallenge(const std::string &verification) {
    // Initialize the SHA-256 hash algorithm
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        // Handle error
        return "";
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        // Handle error
        CryptReleaseContext(hProv, 0);
        return "";
    }

    // Hash the input data
    if (!CryptHashData(hHash, reinterpret_cast<const BYTE*>(verification.data()), verification.length(), 0)) {
        // Handle error
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }

    // Get the hash value size
    DWORD hashSize;
    DWORD dataSize = sizeof(DWORD);
    if (!CryptGetHashParam(hHash, HP_HASHSIZE, reinterpret_cast<BYTE*>(&hashSize), &dataSize, 0)) {
        // Handle error
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }

    // Allocate memory for the hash value
    std::string digest(hashSize, 0);

    // Get the hash value
    if (!CryptGetHashParam(hHash, HP_HASHVAL, reinterpret_cast<BYTE*>(&digest[0]), &hashSize, 0)) {
        // Handle error
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }

    // Release resources
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    // Encode the hash value in Base64
    DWORD base64Size = 0;
    if (!CryptBinaryToStringA(reinterpret_cast<const BYTE*>(&digest[0]), hashSize,
                              CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &base64Size)) {
        // Handle error
        return "";
    }

    std::string encodedDigest(base64Size - 1, 0);
    if (!CryptBinaryToStringA(reinterpret_cast<const BYTE*>(&digest[0]), hashSize,
                              CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &encodedDigest[0], &base64Size)) {
        // Handle error
        return "";
    }

    // Convert to URL-safe Base64
    std::replace(encodedDigest.begin(), encodedDigest.end(), '+', '-');
    std::replace(encodedDigest.begin(), encodedDigest.end(), '/', '_');

    // Remove padding characters '='
    size_t padding = encodedDigest.find_last_not_of('=');
    if (padding != std::string::npos)
        encodedDigest.resize(padding + 1);

    return encodedDigest;
}

bool Authenticator::TestConnection(const std::string& ip, const int& timeout) {
    DWORD ipAddr = inet_addr(ip.c_str());
    if (ipAddr == INADDR_NONE) {
        return false;
    }

    HANDLE hIcmp = IcmpCreateFile();
    if (hIcmp == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Prepare the ICMP Echo Request packet
    const int dataSize = 32;  // Adjust as needed
    char icmpData[dataSize] = {0};  // Adjust the data as needed

    // Send ICMP Echo Request
    DWORD replySize = sizeof(ICMP_ECHO_REPLY) + dataSize;
    char* icmpReply = new char[replySize];
    PICMP_ECHO_REPLY pEchoReply = reinterpret_cast<PICMP_ECHO_REPLY>(icmpReply);

    DWORD result = IcmpSendEcho(hIcmp, ipAddr, icmpData, dataSize, nullptr, pEchoReply, replySize, timeout);
    IcmpCloseHandle(hIcmp);

    delete[] icmpReply;

    if (result > 0) {
        // Successfully received ICMP Echo Reply
        return true;
    } else {
        // Failed to receive ICMP Echo Reply
        return false;
    }
}


#else
// Linux-specific code
std::string Authenticator::CreateChallenge(const std::string &verification) {
    // Use OpenSSL to calculate SHA-256 hash
    CryptoPP::SHA256 hash;
    std::string digest;
    CryptoPP::StringSource(verification, true, new CryptoPP::HashFilter(hash, new CryptoPP::Base64URLEncoder(new CryptoPP::StringSink(digest))));
    return digest;
}

// Unix-specific code to calculate checksum
uint16_t checksum(uint16_t* data, size_t length) {
    uint32_t sum = 0;
    while (length > 1) {
        sum += *data++;
        length -= 2;
    }

    if (length > 0)
        sum += *reinterpret_cast<uint8_t*>(data);

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return static_cast<uint16_t>(~sum);
}

// Unix-specific code to test connection via ICMP ping
bool Authenticator::TestConnection(const std::string& ip, const int& timeout) {
    // If not root, return true
    if (getuid() != 0) {
        std::cout << "Warning: Not running as root. Skipping ping test." << std::endl;
        return true;
    }
    // Prepare ICMP packet
    const int dataSize = 32;  // Adjust as needed
    char icmpData[dataSize] = {0};  // Adjust the data as needed

    // Create socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        return false;
    }

    // Set timeout
    struct timeval tv{};
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        close(sock);
        return false;
    }

    // Prepare ICMP header
    auto* icmpHeader = reinterpret_cast<struct icmp*>(icmpData);
    icmpHeader->icmp_type = ICMP_ECHO;
    icmpHeader->icmp_code = 0;
    icmpHeader->icmp_seq = 0;
    icmpHeader->icmp_cksum = 0;
    icmpHeader->icmp_id = htons(getpid());

    // Calculate checksum
    icmpHeader->icmp_cksum = checksum(reinterpret_cast<uint16_t*>(icmpData), dataSize);

    // Prepare destination address
    struct sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = 0;
    destAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    // Send ICMP packet
    ssize_t bytesSent = sendto(sock, icmpData, dataSize, 0, reinterpret_cast<struct sockaddr*>(&destAddr), sizeof(destAddr));
    if (bytesSent < 0) {
        close(sock);
        return false;
    }

    // Receive ICMP reply
    char icmpReply[dataSize + sizeof(struct iphdr)];
    struct sockaddr_in replyAddr{};
    socklen_t replyAddrLen = sizeof(replyAddr);
    ssize_t bytesReceived = recvfrom(sock, icmpReply, sizeof(icmpReply), 0, reinterpret_cast<struct sockaddr*>(&replyAddr), &replyAddrLen);
    close(sock);
    if (bytesReceived < 0) {
        return false;
    }

    // Check if the reply is valid
    auto* ipHeader = reinterpret_cast<struct ip*>(icmpReply);
    auto* icmpReplyHeader = reinterpret_cast<struct icmp*>(icmpReply + (ipHeader->ip_hl << 2));
    if (icmpReplyHeader->icmp_type == ICMP_ECHOREPLY && icmpReplyHeader->icmp_id == htons(getpid())) {
        // Successfully received ICMP Echo Reply
        return true;
    } else {
        // Failed to receive ICMP Echo Reply
        return false;
    }

}

#endif
