#include <iostream>
#include "DeviceWindow.h"

DeviceWindow::DeviceWindow(const std::string &deviceName, nlohmann::json& deviceInfo) {
    this->deviceName = deviceName;
    this->device = deviceInfo;
    this->setWindowTitle(deviceName.c_str());
}

const QMetaObject* DeviceWindow::metaObject() const {
    return QWidget::metaObject();
}

void* DeviceWindow::qt_metacast(const char *className) {
    return QWidget::qt_metacast(className);
}

int DeviceWindow::qt_metacall(QMetaObject::Call call, int id, void **args) {
    return QWidget::qt_metacall(call, id, args);
}