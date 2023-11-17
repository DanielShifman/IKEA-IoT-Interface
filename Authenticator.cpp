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
    CryptoPP::SHA256 sha256;
    std::string digest;

    CryptoPP::HashFilter filter(sha256);
    filter.Attach(new CryptoPP::StringSink(digest));
    filter.Put(reinterpret_cast<const unsigned char*>(verification.data()), verification.length());
    filter.MessageEnd();

    CryptoPP::Base64URLEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(digest));
    encoder.Put(reinterpret_cast<const unsigned char*>(digest.data()), digest.length());
    encoder.MessageEnd();

    // Convert to URL-safe Base64
    std::replace(digest.begin(), digest.end(), '+', '-');
    std::replace(digest.begin(), digest.end(), '/', '_');

    // Remove padding characters '='
    size_t padding = digest.find_last_not_of('=');
    if (padding != std::string::npos)
        digest.resize(padding + 1);

    return digest;
}

// Linux-specific code to test connection via ICMP ping
bool Authenticator::TestConnection(const std::string& ip, const int& timeout) {
    const int maxBufferSize = 2048; // Adjust as needed
    const int icmpHeaderOffset = 20; // Assuming a standard IP header size, adjust if needed

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    struct icmphdr hdr{};
    hdr.type = ICMP_ECHO;
    hdr.un.echo.id = 0;
    hdr.un.echo.sequence = 0;
    hdr.checksum = checksum(reinterpret_cast<uint16_t*>(&hdr), sizeof(hdr));

    ssize_t sentBytes = sendto(sockfd, &hdr, sizeof(hdr), 0, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (sentBytes <= 0) {
        perror("sendto");
        close(sockfd);
        return false;
    }

    struct timeval tv_out{};
    tv_out.tv_sec = timeout / 1000;
    tv_out.tv_usec = (timeout % 1000) * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));

    struct sockaddr_in r_addr{};
    socklen_t len = sizeof(r_addr);
    char buffer[maxBufferSize];

    ssize_t receivedBytes = recvfrom(sockfd, buffer, maxBufferSize, 0, reinterpret_cast<struct sockaddr*>(&r_addr), &len);
    if (receivedBytes <= 0) {
        perror("recvfrom");
        close(sockfd);
        return false;
    }

    if (receivedBytes < icmpHeaderOffset + sizeof(struct icmphdr)) {
        // Insufficient data received
        close(sockfd);
        return false;
    }

    struct icmphdr rcv_hdr{};
    memcpy(&rcv_hdr, buffer + icmpHeaderOffset, sizeof(rcv_hdr));

    const int echoReplyType = 0;
    close(sockfd);
    return rcv_hdr.type == echoReplyType;
}

#endif
