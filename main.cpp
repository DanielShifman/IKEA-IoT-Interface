#include <iostream>
#include "Config.h"
#include "Devices.h"
#include "Authenticator.h"

using namespace std;

int main(int argc, char** argv) {
    try {
        Config config;
        Devices devices;
        try {
            config = Config("../config.json");
        } catch (std::exception &e) {
            if (strcmp(e.what(), "Could not open file.") == 0) {
                cout << "Config file not found. Creating new config file." << endl;
                cout << "Enter dirigera IP: ";
                cin >> config.ip;
                cout << "Enter dirigera port: ";
                cin >> config.port;
                config.Save();
            } else {
                throw std::runtime_error(std::string(e.what()));
            }
        }

        try {
            devices = Devices("../devices.json");
        } catch (std::exception &e) {
            if (strcmp(e.what(), "Could not open file.") == 0) {
                cout << "Devices file not found. Creating new devices file." << endl;
                devices.Save();
            } else {
                throw std::runtime_error(std::string(e.what()));
            }
        }

        //cout << config.toString() << endl;
        //cout << devices.toString() << endl;
        int pingTimeout = argc <= 1 ? 1000 : 500;
        bool connection = Authenticator::TestConnection(config.ip, pingTimeout);
        while (!connection) {
            char userIn;
            cout << "Cannot connect to server at "<< config.ip << ":" << config.port <<". Please select an option:\n 1. Enter a different IP address\n 2. Enter a different port\n 3. All of the above.\n 4. Exit\n\n";
            cin >> userIn;
            switch (userIn) {
                case '1':
                    cout << "Enter dirigera IP: ";
                    cin >> config.ip;
                    connection = Authenticator::TestConnection(config.ip, pingTimeout);
                    continue;
                case '2':
                    cout << "Enter dirigera port: ";
                    cin >> config.port;
                    connection = Authenticator::TestConnection(config.ip, pingTimeout);
                    continue;
                case '3':
                    cout << "Enter dirigera IP: ";
                    cin >> config.ip;
                    cout << "Enter dirigera port: ";
                    cin >> config.port;
                    connection = Authenticator::TestConnection(config.ip, pingTimeout);
                    continue;
                case '4':
                    return 0;
                default:
                    cout << "Invalid input. Please try again." << endl;
                    continue;
            }
        }
        string token = Authenticator::GetToken(config.ip, stoi(config.port), config.authCode, config.verification, 0);
        cout << "Token: " << token << endl;
    } catch (std::exception &e) {
        cout << e.what() << endl;
    }
    return 0;
}