#ifndef SMARTHUB_DEVICEWINDOW_H
#define SMARTHUB_DEVICEWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "Config.h"
#include "Devices.h"

class DeviceWindow : public QWidget {
Q_OBJECT
private:
    std::string deviceName;
    nlohmann::json device;
public:
    DeviceWindow(const std::string& deviceName, nlohmann::json& deviceInfo);
};


#endif //SMARTHUB_DEVICEWINDOW_H
