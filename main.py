import base64
import ctypes
import hashlib
import json
import os
import socket
import string
import random

from PyQt5 import QtGui
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout
import sys
import requests

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
    print('Response: ' + r.text)
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
    print('Response: ' + r.text)
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
    print('Token: ' + token)
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


def store_device_id(devs: dict):
    devices = {}
    for dev in devs:
        devices[(dev['attributes'])['customName']] = dev['id']
    with open('devices.json', 'w') as f:
        json.dump(devices, f, indent=4)


def repl(ip: str, port: str, token: str, devs: list):
    valid_commands = sorted([
        'status', 'home', 'devices', 'device', 'identify', 'exit', 'help', 'reload devices', 'list ids'
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
        layout = QVBoxLayout()
        self.setWindowTitle(f"{dev_name} - Interface")
        self.create_id_button(layout)
        self.setLayout(layout)

    def identify(self):
        identify_device(self.config['dirigera_ip'], self.config['dirigera_port'], self.config['token'],
                        self.devs[self.dev_name])

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


def main():
    requests.packages.urllib3.disable_warnings()
    global freshFlag
    freshFlag = False
    config = load_config()
    # check_firmware_update(config['dirigera_ip'], config['dirigera_port'], config['token'])
    devs: dict = load_devs(config)
    # Loading Complete
    # repl(config['dirigera_ip'], config['dirigera_port'], config['token'], devs)
    run_gui(config, devs)


if __name__ == '__main__':
    main()
