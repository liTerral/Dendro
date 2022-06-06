from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QLabel, QMainWindow, QPushButton, QTextEdit
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice
import sys

class Window(QMainWindow):
    def __init__(self):
        super(Window, self).__init__()

        self.setWindowTitle('config prog')
        self.setGeometry(350, 200, 460, 550)

def application():
    app = QApplication(sys.argv)
    window = Window()

    window.show()
    sys.exit(app.exec_())


def onRead():
    rx = serial.readAll()
    print(str(rx, 'utf-8').strip())

serial = QSerialPort()
serial.setBaudRate(9600)

portList = []

ports = QSerialPortInfo.availablePorts()

for port in ports:
    portList.append(port.description())

print(portList)

serial.setPortName('\\\\.\\COM3')
serial.open(QIODevice.ReadWrite)

serial.readyRead.connect(onRead)


application()