#include "DeviceWindow.h"

DeviceWindow::DeviceWindow(const std::string &deviceName, nlohmann::ordered_json& deviceInfo, Dirigera& dirigera) {
    this->deviceName = deviceName;
    this->device = deviceInfo;
    this->dirigera = &dirigera;
    this->setWindowTitle((deviceName + " Window").c_str());
    this->setObjectName((deviceName + " Window").c_str());

    auto* layout = new QGridLayout();
    this->createIdButton(layout);
    this->setLayout(layout);

    // List all attributes of the device with their current values as buttons next to them
    std::array<std::string, 1> buttonableAttributes = {"isOn"};
    std::array<double, 3> HSL = {-1, -1, 45};
    bool HSLMade = false;
    // iterate through the device's attributes
    for (auto&[attribute, value] : deviceInfo["attributes"].items()) {
        QWidget* valueWidget;
        if (HSL[0] != -1 && HSL[1] != -1 && !HSLMade) {
            // Create a new horizontal layout for each attribute
            auto* lbLayout = new QHBoxLayout();
            // Create a label for the attribute
            auto* label = new QLabel("Colour");
            lbLayout->addWidget(label);
            // HSL to RGB
            std::array<double, 3> HLS = {HSL[0]/360.0, HSL[2]/100.0, HSL[1]};
            this->RGBarr = HLStoRGB({HLS[0], HLS[1], HLS[2]});
            // RGB to hex
            for (auto& colour : this->RGBarr) {
                int colorInt = static_cast<int>(colour * 255);
                std::stringstream stream;
                stream << std::hex << colorInt;
                std::string result(stream.str());
                if (result.length() == 1) {
                    result = std::string("0").append(result);
                }
                this->RGBHex += result;
            }
            // Display colour as a button
            valueWidget = new QPushButton();
            valueWidget->setStyleSheet(("background-color: #" + this->RGBHex).c_str());
            auto* pBtn = (QPushButton*)(valueWidget);
            QWidget::connect(pBtn, &QPushButton::clicked, [this, pBtn] {
                this->pickColour(pBtn);
            });
            lbLayout->addWidget(valueWidget);
            layout->addLayout(lbLayout, layout->rowCount(), 0);
            HSLMade = true;
        }
        if (attribute == "customName" || attribute == "customImage") continue;
        if (attribute == "colorHue") HSL[0] = static_cast<float>(value);
        if (attribute == "colorSaturation") HSL[1] = static_cast<float>(value);
        // Create a new horizontal layout for each attribute
        auto* lbLayout = new QHBoxLayout();
        // Add the attribute as a label
        auto* label = new QLabel(camelCaseToWords(attribute));
        lbLayout->addWidget(label);

        // If the attribute is buttonable, widget is a button
        // If the attribute is lightLevel, widget is a slider
        // Otherwise, widget is a label
        if (std::find(buttonableAttributes.begin(), buttonableAttributes.end(), attribute) != buttonableAttributes.end()) {
            valueWidget = new QPushButton(value.dump().c_str());
            if (value == true) {
                valueWidget->setStyleSheet("border: 2px solid green; border-radius: 5px;");
            } else {
                valueWidget->setStyleSheet("border: 2px solid red; border-radius: 5px;");
            }
            auto* pBtn = (QPushButton*)(valueWidget);
            QObject::connect(pBtn, &QPushButton::clicked, [this, attribute, pBtn] {
                if (pBtn->text() == "true") {
                    pBtn->setText("false");
                    pBtn->setStyleSheet("border: 2px solid red; border-radius: 5px;");
                    this->dirigera->setDeviceAttribute(this->device["id"], attribute, false);
                } else {
                    pBtn->setText("true");
                    pBtn->setStyleSheet("border: 2px solid green; border-radius: 5px;");
                    this->dirigera->setDeviceAttribute(this->device["id"], attribute, true);
                }
            });
            lbLayout->addWidget(valueWidget);
        } else if (attribute == "lightLevel" and this->device["type"] == "light") {
            valueWidget = new lightLevelSlider(Qt::Horizontal);
            auto* slider = (QSlider*)(valueWidget);
            slider->setRange(1, 100);
            slider->setSingleStep(10);
            slider->setValue(static_cast<int>(value));
            slider->setTickInterval(10);
            slider->setTickPosition(QSlider::TicksBothSides);
            auto* sliderLabel = new QLabel(value.dump().c_str());
            QObject::connect(slider, &QSlider::sliderReleased, [this, attribute, slider] {
                this->dirigera->setDeviceAttribute(this->device["id"], attribute, slider->value());
            });
            QObject::connect(slider, &QSlider::sliderMoved, [attribute, sliderLabel](int newValue) {
                sliderLabel->setText(std::to_string(newValue).c_str());
            });
            QObject::connect(slider, &QSlider::valueChanged, [attribute, sliderLabel](int newValue) {
                sliderLabel->setText(std::to_string(newValue).c_str());
            });
            auto* sliderLayout = new QHBoxLayout();
            sliderLayout->addWidget(sliderLabel);
            lbLayout->addSpacerItem(new QSpacerItem(this->width()/7, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
            sliderLayout->addWidget(valueWidget);

            lbLayout->addLayout(sliderLayout);
        } else {
            // Add the value as a label without quotes via regex
            valueWidget = new QLabel(std::regex_replace(value.dump(), std::regex("\""), "").c_str());
            lbLayout->addWidget(valueWidget);
        }

        layout->addLayout(lbLayout, layout->rowCount(), 0);
    }
    this->resize(int(this->width()*0.5), this->height());
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


void DeviceWindow::createIdButton(QLayout *layout) {
    auto* idButton = new QPushButton("Identify");
    idButton->setToolTip(("Identify " + this->deviceName).c_str());
    QObject::connect(idButton, &QPushButton::clicked, [this] {
            dirigera->identifyDevice(this->device["id"]);
    });
    if (this->device["type"] == "controller") {
        idButton->setEnabled(false);
    }
    layout->addWidget(idButton);
}

std::array<double, 3> DeviceWindow::HLStoRGB(std::array<double, 3> HLS) {
    double h = HLS[0];
    double l = HLS[1];
    double s = HLS[2];

    double r, g, b;

    if (s == 0.0) {
        // Achromatic
        r = g = b = l;
    } else {
        auto hue2rgb = [](double p, double q, double t) {
            if (t < 0.0) t += 1.0;
            if (t > 1.0) t -= 1.0;
            if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
            if (t < 1.0 / 2.0) return q;
            if (t < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
            return p;
        };

        double q = (l < 0.5) ? (l * (1.0 + s)) : (l + s - l * s);
        double p = 2.0 * l - q;

        r = hue2rgb(p, q, h + 1.0 / 3.0);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0 / 3.0);
    }
    return {r, g, b};
}

char* DeviceWindow::camelCaseToWords(const std::string &camelCase) {
    // replace "color" with "colour" case invariantly using regex_replace
    std::string str = std::regex_replace(camelCase, std::regex("color", std::regex_constants::icase), "colour");
    // Create a regular expression object for finding words in camelCase
    std::regex pattern(R"(([a-z]+|[A-Z][a-z]*))");

    // Create a sregex_iterator object to iterate over matches
    std::sregex_iterator iterator(str.begin(), str.end(), pattern);
    std::sregex_iterator endIterator;

    // Concatenate words with a space
    std::string result;
    for (; iterator != endIterator; ++iterator) {
        result += iterator->str() + " ";
    }

    if (!result.empty()) {
        result[0] = std::toupper(result[0]);
    }

    // Convert the result to a char array
    char* resultCharArray = new char[result.size() + 1];
    std::copy(result.begin(), result.end(), resultCharArray);
    resultCharArray[result.size()] = '\0';

    return resultCharArray;
}

void DeviceWindow::pickColour(QPushButton *pBtn) {
    colourResult colour = this->openColourPicker();
    if (colour.HSV[0] != -1 && colour.HSV[1] != -1 && colour.HSV[2] != -1) {
        this->RGBarr = DeviceWindow::arr3i2d(colour.RGB);
        for (auto& RGBComponent : this->RGBarr) {
            RGBComponent /= 255;
        }
        std::string newRGBHex;
        for (auto& RGBComponent : colour.RGB) {
            // Convert each RGB component to hex
            std::stringstream stream;
            stream << std::hex << RGBComponent;
            std::string result(stream.str());
            if (result.length() == 1) {
                result = std::string("0").append(result);
            }
            newRGBHex += result;
        }
        this->RGBHex = newRGBHex;
        pBtn->setStyleSheet(("background-color: #" + newRGBHex).c_str());
        try {
            std::string jsonStr =
            "["
                "{"
                    "\"attributes\": {"
                        "\"colorHue\": " + std::to_string(colour.HSV[0]) + ","
                        "\"colorSaturation\": " + std::to_string(colour.HSV[1]/255.0) +
                    "}"
                "}"
            "]";
            this->dirigera->setDeviceAttributes(this->device["id"], jsonStr);
            this->RGBHex = newRGBHex;
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
}

DeviceWindow::colourResult DeviceWindow::openColourPicker() {
    // Scale RGBarr to 0-255
    int rgbArr[3] = {static_cast<int>(this->RGBarr[0] * 255), static_cast<int>(this->RGBarr[1] * 255), static_cast<int>(this->RGBarr[2] * 255)};
    auto colour = QColorDialog::getColor(QColor::fromRgb(rgbArr[0], rgbArr[1], rgbArr[2]), this);
    if (colour.isValid()) {
        std::array<int, 3> HSV = {colour.hue(), colour.saturation(), colour.value()};
        std::array<int, 3> RGB = {colour.red(), colour.green(), colour.blue()};
        return colourResult {HSV, RGB};
    } else {
        return {-1, -1, -1};
    }
}

std::array<double, 3> DeviceWindow::arr3i2d(std::array<int, 3> RGB) {
    return {static_cast<double>(RGB[0]), static_cast<double>(RGB[1]), static_cast<double>(RGB[2])};
}

void DeviceWindow::lightLevelSlider::mousePressEvent(QMouseEvent *event) {
    // Retrieve the QStyleOptionSlider from the style
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    // Check if the click is on the slider thumb
    QRect thumbRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    if (!thumbRect.contains(event->pos())) {
        // Ignore the event if not on the thumb
        event->ignore();
        return;
    }

    // Call the base class implementation for thumb clicks
    QSlider::mousePressEvent(event);
}

void DeviceWindow::lightLevelSlider::wheelEvent(QWheelEvent *event) {
    // Prevent handling mouse wheel events
    event->ignore();
}
