import base64
import colorsys
import ctypes
import hashlib
import json
import os
import re
import socket
import string
import random

from PyQt5 import QtGui
from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QLabel, QGridLayout, QHBoxLayout, \
    QColorDialog, QSlider
from PyQt5.QtCore import Qt
import sys
import requests
from urllib3.exceptions import InsecureRequestWarning

global freshFlag

endpoints = {
    'status': '/v1/hub/status',
    'home': '/v1/home',
    'devices': '/v1/devices',
    'device_set': '/v1/device-set',
    'rooms': '/v1/rooms',
    'scenes': '/v1/scenes',
    'groups': '/v1/groups',
    'activity_log': '/v1/activitylog',
    'users': '/v1/users',
    'firmware_update': '/v1/hub/ota/check',  # PUT
    'authorise': '/v1/oauth/authorize',
    'token': '/v1/oauth/token'
}


def create_challenge(verification: str):
    sha256 = hashlib.sha256()
    sha256.update(verification.encode())
    digest = sha256.digest()
    sha256HashAsBase64 = base64.urlsafe_b64encode(digest).rstrip(b'=').decode('us-ascii')
    return sha256HashAsBase64


def authorise(ip: str, port: str, verification: str):
    url = 'https://' + ip + ':' + port + endpoints['authorise']
    # Get url using requests and ignore invalid SSL certificate
    payload = {
        # 'audience': "192.168.1.87",
        'response_type': 'code',
        'code_challenge_method': 'S256',
        'code_challenge': create_challenge(verification)
    }

    headers = {"Content-Type": "application/x-www-form-urlencoded"}

    r = requests.get(url, params=payload, headers=headers, verify=False)
    # print('Response: ' + r.text)
    auth_code = r.json()['code']
    return auth_code


def load_config():
    # Check if config file exists
    if not os.path.isfile('config.json'):
        # Create config file
        with open('config.json', 'w') as f:
            config = {
                'dirigera_ip': input('Enter dirigera IP: '),
                'dirigera_port': input('Enter dirigera port: '),
                'auth_code': 'None',
                'verification': 'None',
                'token': 'None'
            }
            json.dump(config, f, indent=4)
    with open('config.json', 'r+') as f:
        config = json.load(f)
        dirigera_ip: str = config['dirigera_ip']
        dirigera_port: str = config['dirigera_port']
        try:
            auth_code: str = config['auth_code']
        except KeyError:
            auth_code = "None"
        try:
            verification: str = config['verification']
        except KeyError:
            verification = "None"
        if verification == "None" or auth_code == "None" or freshFlag:
            verification = get_verification()
            config['verification'] = verification
            auth_code = authorise(dirigera_ip, dirigera_port, verification)
            config['auth_code'] = auth_code
            f.seek(0)
            json.dump(config, f, indent=4)
            f.truncate()
        try:
            token: str = config['token']
        except KeyError:
            token = "None"
        if token == "None" or freshFlag:
            input("Press action button on the device and press enter to continue...")
            token = get_token(config['dirigera_ip'], config['dirigera_port'], config['auth_code'],
                              config['verification'])
    return {'dirigera_ip': dirigera_ip, 'dirigera_port': dirigera_port, 'auth_code': auth_code,
            'verification': verification, 'token': token}


def print_config(config):
    for key, value in config.items():
        print(key + ': ' + value)


def get_verification():
    AB = string.ascii_uppercase + string.ascii_lowercase + string.digits + '-._~'
    code = ""
    for _ in range(0, 128):
        code += AB[random.randrange(0, len(AB))]
    return code


def get_token(ip, port, auth_code, verification):
    url = 'https://' + ip + ':' + port + endpoints['token']
    headers = {"Content-Type": "application/x-www-form-urlencoded"}
    payload = str(
        'grant_type=authorization_code&code=' + auth_code + '&name=' + socket.gethostname() + '&code_verifier=' + verification)
    r = requests.post(url, data=payload, headers=headers, verify=False)
    # print('Response: ' + r.text)
    try:
        token = r.json()['access_token']
    except KeyError:
        print('Error: ' + r.json()['error'])
        print('Description: ' + r.json()['error'])
        exit(1)
    # Save token to config
    with open('config.json', 'r+') as f:
        config = json.load(f)
        config['token'] = token
        f.seek(0)
        json.dump(config, f, indent=4)
        f.truncate()
    # print('Token: ' + token)
    return token


def status(ip, port, token):
    url = 'https://' + ip + ':' + port + endpoints['status']
    headers = {"Authorization": "Bearer " + token}
    r = requests.get(url, headers=headers, verify=False)
    # Print entire response
    try:
        status_json = r.text
    except KeyError:
        print('Error: ' + r.json()['error'])
        return None
    return status_json


def home(ip, port, token):
    url = 'https://' + ip + ':' + port + endpoints['home']
    headers = {"Authorization": "Bearer " + token}
    r = requests.get(url, headers=headers, verify=False)
    # Print entire response
    try:
        home_json = r.text
    except KeyError:
        print('Error: ' + r.json()['error'])
        return None
    return home_json


def check_firmware_update(ip, port, token):
    url = 'https://' + ip + ':' + port + endpoints['firmware_update']
    headers = {"Authorization": "Bearer " + token}
    r = requests.put(url, headers=headers, verify=False)
    # Print entire response
    try:
        stat = r.status_code
        if stat != 202:
            print('Error: ' + "Cannot check for firmware update")
            return None
    except KeyError:
        print('Error: ' + r.json()['error'])


def list_devices(ip, port, token):
    url = 'https://' + ip + ':' + port + endpoints['devices']
    headers = {"Authorization": "Bearer " + token}
    r = requests.get(url, headers=headers, verify=False)
    # Print entire response
    try:
        # read response text as json
        devices_json = r.json()
    except KeyError:
        print('Error: ' + r.json()['error'])
        return None
    return devices_json


def get_device(ip, port, token, device_id):
    url = 'https://' + ip + ':' + port + endpoints['devices'] + '/' + device_id
    headers = {"Authorization": "Bearer " + token}
    r = requests.get(url, headers=headers, verify=False)
    # Print entire response
    try:
        device_json = r.text
    except KeyError:
        print('Error: ' + r.json()['error'])
        return None
    return device_json


def identify_device(ip, port, token, device_id):
    url = 'https://' + ip + ':' + port + endpoints['devices'] + '/' + device_id + '/identify'
    headers = {"Authorization": "Bearer " + token}
    r = requests.put(url, headers=headers, verify=False)
    # Print entire response
    try:
        stat = r.status_code
        if stat != 202:
            print('Error: ' + "Cannot identify device " + str(device_id))
            return None
    except KeyError:
        print('Error: ' + r.json()['error'])


def set_device_attribute(ip: str, port: str, token: str, device_id: str, attribute_name: str, attribute_value,
                         val_type: str = "string"):
    att_val = None
    if val_type == "bool":
        if type(attribute_value) is bool:
            att_val = attribute_value
        else:
            if attribute_value.lower() == "auto":
                # Get current value of attribute
                device = get_device(ip, port, token, device_id)
                device_json = json.loads(device)
                att_val = not device_json['attributes'][attribute_name]
            elif attribute_value.lower() == "true":
                att_val = True
            else:
                att_val = False
    elif val_type == "float":
        att_val = float(attribute_value)
    else:
        att_val = attribute_value

    attr = [
        {
            "attributes": {
                attribute_name: att_val
            }
        }
    ]
    url = 'https://' + ip + ':' + port + endpoints['devices'] + '/' + device_id
    headers = {"Authorization": "Bearer " + token}
    r = requests.patch(url, headers=headers, verify=False, json=attr)
    print(r.request.body)
    # Print entire response
    try:
        stat = r.status_code
        if stat != 202:
            print('Error: ' + "Cannot set attribute " + attribute_name + " for device " + str(device_id))
            # define a regex string
            err_resp = r"\{\"error\"\:\"Error\",\"message\"\:\"(.*)\"\}"
            # find all matches
            matches = re.findall(err_resp, r.text)
            # print the matches
            if len(matches) > 0:
                print(matches[0])
            return None
    except Exception:
        print('Error: ' + r.json()['error'])


def set_device_attributes(ip: str, port: str, token: str, device_id: str, attr: json):
    url = 'https://' + ip + ':' + port + endpoints['devices'] + '/' + device_id
    headers = {"Authorization": "Bearer " + token}
    r = requests.patch(url, headers=headers, verify=False, json=attr)
    print(r.request.body)
    # Print entire response
    try:
        stat = r.status_code
        if stat != 202:
            print('Error: ' + "Cannot set attributes " + str(attr) + " for device " + str(device_id))
            # define a regex string
            err_resp = r"\{\"error\"\:\"Error\",\"message\"\:\"(.*)\"\}"
            # find all matches
            matches = re.findall(err_resp, r.text)
            # print the matches
            if len(matches) > 0:
                print(matches[0])
            return None
    except Exception:
        print('Error: ' + r.json()['error'])


def store_device_id(devs: dict):
    devices = {}
    for dev in devs:
        devices[(dev['attributes'])['customName']] = dev['id']
    with open('devices.json', 'w') as f:
        json.dump(devices, f, indent=4)


def repl(ip: str, port: str, token: str, devs: list):
    valid_commands = sorted([
        'status', 'home', 'devices', 'device', 'identify', 'exit', 'help', 'reload devices', 'list ids', 'set attribute'
    ])
    while True:
        command = input('Enter command: ')
        if command not in valid_commands:
            try:
                command = valid_commands[int(command)]
            except Exception:
                print('Invalid command')
                continue
        if command == 'help':
            for i in range(0, len(valid_commands)):
                print(f"{i}.\t{valid_commands[i]}")
        if command == 'exit':
            exit(0)
        if command == 'status':
            print(status(ip, port, token))
        if command == 'home':
            print(home(ip, port, token))
        if command == 'reload devices':
            print(list_devices(ip, port, token))
        if command == 'devices':
            print(devs)
        if command == 'device':
            device_name: str = input('Enter device name: ')
            print(get_device(ip, port, token, devs[device_name]))
        if command == 'identify':
            device_name: str = input('Enter device name: ')
            identify_device(ip, port, token, devs[device_name])
        if command == 'list ids':
            for key in devs:
                print(key)
        if command == 'set attribute':
            device_name: str = input('Enter device name: ')
            attribute_name: str = input('Enter attribute name: ')
            attribute_value: str = input('Enter attribute value: ')
            att_type: str = input('Enter attribute type: ')
            set_device_attribute(ip, port, token, devs[device_name], attribute_name, attribute_value, att_type)


def load_devs(config: dict):
    with open('devices.json', 'r') as f:
        devs: dict = json.load(f)
        if devs == {}:
            devs = list_devices(config['dirigera_ip'], config['dirigera_port'], config['token'])
            store_device_id(devs)
    return dict(sorted(devs.items()))


def set_windows_icon():
    # If OS is Windows, set icon
    if sys.platform != 'win32': return
    app_id = u'IKEA Smart Device Interface'
    ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(app_id)


class DeviceWindow(QWidget):
    def __init__(self, config: dict, devs: dict, dev_name: str, dev_list: list):
        super().__init__()
        self.dev_list = dev_list
        self.config = config
        self.devs = devs
        self.dev_name = dev_name
        self.dev_id = devs[dev_name]
        self.dev_obj = self.get_dev_obj()
        layout = QGridLayout()
        self.setWindowTitle(f"{dev_name} - Interface")
        self.create_id_button(layout)
        self.setLayout(layout)
        # List all attributes of the device with their current values as buttons next to them
        buttonable_attributes = ["isOn"]
        HSL = (None, None, 45)
        HSLMade = False
        for key, value in self.dev_obj['attributes'].items():
            if HSL[0] is not None and HSL[1] is not None and HSL[2] is not None and not HSLMade:
                # Create a new horizontal layout for each attribute
                lb_layout = QHBoxLayout()
                # Add the key as a label
                label = QLabel("Colour")
                lb_layout.addWidget(label)
                # HSL to RGB
                HLS = (HSL[0] / 360.0, HSL[2] / 100.0, HSL[1])
                RGB = colorsys.hls_to_rgb(HLS[0], HLS[1], HLS[2])
                # RGB to HEX
                self.RGB = tuple(int(x * 255) for x in RGB)
                RGB_hex = '#%02x%02x%02x' % self.RGB
                # Display colour as a button
                widget = QPushButton()
                widget.setStyleSheet("background-color: " + RGB_hex)
                widget.clicked.connect(
                    lambda checked, dev_name=self.dev_name, attribute_name="colorHue", value=HSL[0]: self.pick_colour(
                        dev_name, self.RGB))
                lb_layout.addWidget(widget)
                layout.addLayout(lb_layout, layout.rowCount(), 0)
                HSLMade = True
            if key == "customName" or key == "customImage":
                continue
            if key == "colorHue":
                HSL = (float(value), HSL[1], HSL[2])
            if key == "colorSaturation":
                HSL = (HSL[0], float(value), HSL[2])
            # Create a new horizontal layout for each attribute
            lb_layout = QHBoxLayout()
            # Add the key as a label
            firstWord = re.findall(r'^[^A-Z]*', str(key).replace("color", "colour"))
            firstWord[0] = str(firstWord[0]).capitalize()
            capWords = re.findall(r'[A-Z](?:[a-z]+|[A-Z]*(?=[A-Z]|$))', str(key).replace("color", "colour"))
            capWords = list(map(lambda x: x.lower(), capWords))
            words = firstWord + capWords
            words = " ".join(words)
            label = QLabel(words)
            lb_layout.addWidget(label)
            if key == "lightLevel" and self.dev_obj["type"] == "light":
                slider_widget = QWidget()
                slider_layout = QHBoxLayout()

                widget = QSlider(Qt.Horizontal)
                widget.setRange(1, 100)
                widget.setSingleStep(10)
                widget.setValue(int(value))
                widget.setTickInterval(10)
                widget.setTickPosition(QSlider.TicksBothSides)

                slider_label = QLabel(str(value))

                self.connect_set_light_level(widget, slider_label)

                # Add a stretchable space before the label to push it to the right
                slider_layout.addWidget(widget)
                slider_layout.addStretch(1)
                slider_layout.addWidget(slider_label)

                slider_widget.setLayout(slider_layout)
                lb_layout.addWidget(slider_widget)
            # Add the value as a button if it is buttonable
            if key in buttonable_attributes:
                widget = QPushButton(str(value))
                if str(value) == "True":
                    widget.setStyleSheet("border: 2px solid green; border-radius: 5px; box-shadow: 2px 2px 2px grey;")
                elif str(value) == "False":
                    widget.setStyleSheet("border: 2px solid red; border-radius: 5px; box-shadow: 2px 2px 2px grey;")
                self.connect_button_clicked(widget, self.dev_name, key)
            elif key == "lightLevel" and self.dev_obj["type"] == "light":
                pass
            else:
                widget = QLabel(str(value))
                if key == "colorHue":
                    self.hueLabel = widget
                if key == "colorSaturation":
                    self.satLabel = widget
            lb_layout.addWidget(widget)
            # Add the horizontal layout to the main layout
            layout.addLayout(lb_layout, layout.rowCount(), 0)
        # Set windows size to be a reasonable width relative to the width of the screen
        self.resize(int(self.width() * 0.5), self.height())

    def identify(self):
        identify_device(self.config['dirigera_ip'], self.config['dirigera_port'], self.config['token'],
                        self.devs[self.dev_name])

    def connect_button_clicked(self, button, dev_name, attribute_name):
        def on_button_clicked():
            value = button.text()
            if value == "False":
                button.setStyleSheet("border: 2px solid green; border-radius: 5px; box-shadow: 2px 2px 2px grey;")
            elif value == "True":
                button.setStyleSheet("border: 2px solid red; border-radius: 5px; box-shadow: 2px 2px 2px grey;")
            self.set_attribute(dev_name, attribute_name, value, button)

        button.clicked.connect(on_button_clicked)

    def connect_set_light_level(self, slider, label):
        def on_value_changed(val):
            self.set_attribute(self.dev_name, "lightLevel", val, None)
            # Update slider text to show current value
            label.setText(str(val))

        slider.valueChanged.connect(on_value_changed)

    def create_id_button(self, layout: QVBoxLayout):
        id_button = QPushButton('Identify')
        id_button.clicked.connect(lambda: self.identify())
        if (self.dev_obj["type"]) == "controller":
            id_button.setEnabled(False)
        layout.addWidget(id_button)

    def get_dev_obj(self):
        for dev in self.dev_list:
            if dev['id'] == self.dev_id:
                return dev

    def set_attribute(self, dev_name, attribute_name, current_value, widget):
        att_type = None
        new_value = None
        if attribute_name == "isOn":
            att_type = "bool"
            if current_value.lower() == "true":
                new_value = False
            else:
                new_value = True
        else:
            att_type = "string"
            new_value = current_value
        print(f"Setting {attribute_name} for {dev_name} (currently {current_value}) to {new_value}")
        set_device_attribute(self.config['dirigera_ip'], self.config['dirigera_port'], self.config['token'],
                             self.dev_id, attribute_name, new_value, att_type)
        print(type(widget))
        if type(widget) is QPushButton:
            # Update the value of the button
            widget.setText(str(new_value))

    def open_colour_picker(self, initial_hex: tuple):
        # Convert RGB hex string to R, G, B values
        colour = QColorDialog.getColor(parent=self,
                                       initial=QColor.fromRgb(initial_hex[0], initial_hex[1], initial_hex[2]))
        if colour.isValid():
            print(colour.name())
            print(colour.hue(), colour.saturation(), colour.value())
            return colour.hue(), colour.saturation(), colour.value(), colour.name()
        else:
            return None

    def pick_colour(self, dev_name, curr_rgb):
        # Open a colour picker window
        print("Picking colour for " + dev_name)
        HSV = self.open_colour_picker(self.RGB)
        if HSV is not None:
            # Iterate through the layout and find the hue and saturation buttons
            for i in range(1, self.layout().count()):
                lb_layout = self.layout().itemAt(i).layout()
                if lb_layout.itemAt(0).widget().text() == "Colour":
                    lb_layout.itemAt(1).widget().setStyleSheet("background-color: " + HSV[3])
                if lb_layout.itemAt(0).widget().text() == "colorHue":
                    lb_layout.itemAt(1).widget().setText(str(HSV[0]))
                if lb_layout.itemAt(0).widget().text() == "colorSaturation":
                    lb_layout.itemAt(1).widget().setText(str(HSV[1] / 255))
            self.send_colour(HSV[0], HSV[1] / 255)
            # Update hue and saturation labels
            self.hueLabel.setText(str(HSV[0]))
            self.satLabel.setText(str(HSV[1] / 255))
            # Update RGB from hex tuple
            self.RGB = tuple(int(HSV[3][i:i + 2], 16) for i in (1, 3, 5))

    def send_colour(self, hue_val: float, sat_val: float):
        # Send the colour to the device
        attr = [
            {
                "attributes": {
                    "colorHue": hue_val,
                    "colorSaturation": sat_val,
                }
            }
        ]
        set_device_attributes(self.config['dirigera_ip'], self.config['dirigera_port'], self.config['token'], self.dev_id, attr)


def run_gui(config: dict, devs: dict):
    dev_list = list_devices(config['dirigera_ip'], config['dirigera_port'], config['token'])
    print(dev_list)

    set_windows_icon()
    app = QApplication(sys.argv)

    root_window = QWidget()
    root_window.setWindowTitle('IKEA Smart Device Interface')
    root_window.resize(200, 200)
    root_window.setWindowIcon(QtGui.QIcon('ikea.png'))
    root_window.show()

    # Layout
    layout = QVBoxLayout()

    # Create a button for each device
    dev_butts = {}
    dev_windows = {}

    def dev_button_clicked(dev_name: str):
        # Create a new window for the device
        if dev_name not in dev_windows:
            dev_window = DeviceWindow(config, devs, dev_name, dev_list)
            dev_windows[dev_name] = dev_window
        else:
            dev_windows[dev_name].activateWindow()
        dev_windows[dev_name].show()

    for dev in devs:
        dev_butts[dev] = QPushButton(dev)
        dev_butts[dev].clicked.connect(lambda checked, dev_name=dev: dev_button_clicked(dev_name))
        layout.addWidget(dev_butts[dev])

    root_window.setLayout(layout)
    app.exec()


def main(args=sys.argv[1:]):
    requests.packages.urllib3.disable_warnings(category=InsecureRequestWarning)
    global freshFlag
    freshFlag = False
    config = load_config()
    # check_firmware_update(config['dirigera_ip'], config['dirigera_port'], config['token'])
    devs: dict = load_devs(config)
    # Loading Complete
    if len(args) == 0:
        # repl(config['dirigera_ip'], config['dirigera_port'], config['token'], devs)
        run_gui(config, devs)
    else:
        dev_name = args[0]
        dev_att = args[1]
        new_dev_val = args[2]
        val_type = args[3]
        set_device_attribute(config['dirigera_ip'], config['dirigera_port'], config['token'], devs[dev_name], dev_att,
                             new_dev_val, val_type)


if __name__ == '__main__':
    main()
