#ifndef SMARTHUB_DEVICEWINDOW_H
#define SMARTHUB_DEVICEWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QSlider>
#include <QRect>
#include <QStyle>
#include <QPoint>
#include <QWheelEvent>
#include <QColorDialog>
#include <iostream>
#include <regex>
#include <QStyleOptionSlider>
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
    [[maybe_unused]]std::array<double, 3> RGBarr{};

    typedef struct colourResult {
        std::array<int, 3> HSV;
        std::array<int, 3> RGB;
    } colourResult;

    void createIdButton(QLayout* layout);
    static std::array<double, 3> HLStoRGB(std::array<double, 3> HLS);
    static char* camelCaseToWords(const std::string& camelCase);
    void pickColour(QPushButton* pBtn);
    colourResult openColourPicker();
    static std::array<double, 3> arr3i2d(std::array<int, 3> RGB);

    class lightLevelSlider : public QSlider {
    public:
        explicit lightLevelSlider(Qt::Orientation orientation, QWidget* parent = nullptr) : QSlider(orientation, parent) {}

    protected:
        void wheelEvent(QWheelEvent* event) override;

        void mousePressEvent(QMouseEvent* event) override;
    };

public:

    DeviceWindow(const std::string& deviceName, nlohmann::ordered_json& deviceInfo, Dirigera& dirigera);
};


#endif //SMARTHUB_DEVICEWINDOW_H
