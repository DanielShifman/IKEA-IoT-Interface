#include <iostream>
#include <vector>
#include <algorithm>
#include "Config.h"
#include "Devices.h"
#include "Authenticator.h"
#include "Dirigera.h"
#include "DeviceWindow.h"

#define RESOURCE_DIR_PATH "./resources/"

using namespace std;

vector<string> *boolable = new vector<string>{"isOn"};

void run_gui(Devices& devs, Dirigera dirigera, int argc, char** argv) {
    // Initialise application
    QApplication app(argc, argv);

    // Create the root window
    auto* rootWindow = new QWidget();
    rootWindow->setWindowTitle("IKEA Smart Device Interface");
    rootWindow->resize(200,200);
    rootWindow->setWindowIcon(QIcon(RESOURCE_DIR_PATH "ikea.png"));
    rootWindow->show();

    // Create a layout for the root window
    auto* layout = new QVBoxLayout(rootWindow);
    for (const auto&[deviceName, deviceId] : devs.getDevices()) {
        auto* button = new QPushButton(deviceName.c_str(), rootWindow);
        button->setObjectName(deviceName.c_str());
        button->setToolTip(("Open " + deviceName + " window").c_str());
        layout->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, [&, deviceName, deviceId, rootWindow] {
            std::string deviceWindowName = deviceName + " Window";
            //If the device window is not already open, open it. Otherwise, activate it.
            auto* existingWindow = rootWindow->findChild<QWidget *>(deviceWindowName.c_str(), Qt::FindDirectChildrenOnly);
            if (existingWindow == nullptr) {
                nlohmann::ordered_json deviceInfo = dirigera.getDevice(deviceId);
                auto* deviceWindow = new DeviceWindow(deviceName, deviceInfo, dirigera);
                deviceWindow->setParent(rootWindow, Qt::Window);
                deviceWindow->setAttribute(Qt::WA_DeleteOnClose);
                deviceWindow->show();
            } else {
                existingWindow->activateWindow();
            }
        });
    };

    // Set the layout for the root window
    rootWindow->setLayout(layout);

    // Start the application event loop
    QApplication::exec();
}

int main(int argc, char** argv) {
    try {
        Config config;
        Devices devices;
        try {
            config = Config(RESOURCE_DIR_PATH "config.json");
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
            devices = Devices(RESOURCE_DIR_PATH "devices.json");
        } catch (std::exception &e) {
            if (strcmp(e.what(), "Could not open file.") == 0 || strcmp(e.what(), "Empty JSON file.") == 0) {
                cout << "Devices file not found. Creating new devices file." << endl;
                devices.Save();
            } else {
                throw std::runtime_error(std::string(e.what()));
            }
        }

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
        if (config.token.empty()) {
            config.verification = Authenticator::GetVerification();
            config.authCode = Authenticator::Authorise(config.ip, stoi(config.port), config.verification);
            config.token = Authenticator::GetToken(config.ip, stoi(config.port), config.authCode, config.verification, 0);
            config.Save();
        }
        Dirigera dirigera(config.ip, config.port, config.token);
        nlohmann::ordered_json devicesJson = dirigera.getDevices();

        for (auto &device : devicesJson) {
            devices.addDevice(device["attributes"]["customName"], device["id"]);
        }
        if (!devices.isEmpty()) {
            devices.Save();
        }
        if (argc > 1) {
            string deviceName = argv[1];
            string attribute = argv[2];
            string value = argv[3];
            cout << "Setting " << deviceName << " " << attribute << " to " << value << endl;
            if (find(boolable->begin(), boolable->end(), attribute) != boolable->end()) {
                dirigera.setDeviceAttribute(devices.getDevice(deviceName), attribute, bool(stoi(value)));
            } else {
                dirigera.setDeviceAttribute(devices.getDevice(deviceName), attribute, value);
            }
        } else {
            run_gui(devices, dirigera, argc, argv);
        }
    } catch (std::exception &e) {
        cout << e.what() << endl;
    }
    return 0;
}