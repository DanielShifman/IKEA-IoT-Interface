#ifndef SMARTHUB_DEVICEWINDOW_H
#define SMARTHUB_DEVICEWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <iostream>
#include <regex>
#include "Config.h"
#include "Devices.h"
#include "Dirigera.h"

class DeviceWindow : public QWidget {
Q_OBJECT
private:
    std::string deviceName;
    nlohmann::ordered_json device;
    Dirigera* dirigera;
    [[maybe_unused]]std::string RGBHex;

    void createIdButton(QLayout* layout);
    static std::array<double, 3> HLStoRGB(std::array<double, 3> HLS);
    static char* camelCaseToWords(const std::string& camelCase);

public:

    DeviceWindow(const std::string& deviceName, nlohmann::ordered_json& deviceInfo, Dirigera& dirigera);
};


#endif //SMARTHUB_DEVICEWINDOW_H
