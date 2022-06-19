from PyQt5 import QtWidgets, QtCore, QtGui, uic
from PyQt5.QtWidgets import *
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice
import sys, os
import datetime

class Window(QMainWindow):
    def __init__(self):
        super(Window, self).__init__()

        self.serial = QSerialPort()
        self.serial.setBaudRate(9600)
        self.serial.readyRead.connect(self.onRead)

        self.setWindowTitle('Settings')
        scriptDir = os.path.dirname(os.path.realpath(__file__))
        self.setWindowIcon(QtGui.QIcon(scriptDir + os.path.sep +'icon.png'))
        self.resize(305, 370)

        mainLayout = QVBoxLayout()
        tabWidget = QTabWidget(self)

        tabWidget.setGeometry(0, 0, 310, 380)

        self.tabPort = QWidget()
        self.tabParameters = QWidget()
        self.tabData = QWidget()

        tabWidget.addTab(self.tabPort, "Порт")
        tabWidget.addTab(self.tabParameters, "Параметри")
        tabWidget.addTab(self.tabData, "Дані")

        mainLayout.addWidget(tabWidget)


        ##### tab port #####

        self.tabPort.groupBox = QGroupBox('Відкрийте порт, по якому підключений пристрій')

        self.tabPort.comboCom = QComboBox()
        self.checkPorts()
        self.tabPort.comboCom.setMinimumWidth(50)

        self.tabPort.btnUpd = QPushButton()
        self.tabPort.btnUpd.setText('Оновити')
        self.tabPort.btnUpd.clicked.connect(self.checkPorts)

        self.tabPort.btnOpen = QPushButton()
        self.tabPort.btnOpen.setText('Відкрити')
        self.tabPort.btnOpen.clicked.connect(self.openCom)

        self.tabPort.btnClose = QPushButton()
        self.tabPort.btnClose.setText('Закрити')
        self.tabPort.btnClose.clicked.connect(self.closeCom)

        self.tabPort.hbox = QHBoxLayout()
        self.tabPort.hbox.addWidget(self.tabPort.comboCom)
        self.tabPort.hbox.addWidget(self.tabPort.btnOpen)

        self.tabPort.hbox2 = QHBoxLayout()
        self.tabPort.hbox2.addWidget(self.tabPort.btnUpd)
        self.tabPort.hbox2.addWidget(self.tabPort.btnClose)

        self.tabPort.vbox = QVBoxLayout()
        self.tabPort.vbox.addLayout(self.tabPort.hbox)
        self.tabPort.vbox.addLayout(self.tabPort.hbox2)

        self.tabPort.groupBox.setLayout(self.tabPort.vbox)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.tabPort.groupBox)
        mainLayout.addStretch(1)
        self.tabPort.setLayout(mainLayout)

        ###################

        ##### tab param ####

        self.tabParameters.mainLayout = QVBoxLayout()
        
        
        self.tabParameters.fbox = QFormLayout()

        self.tabParameters.lb_tmUpd = QLabel("Час оновлення даних в активному режимі:")
        self.tabParameters.lb_tmPasUpd = QLabel("Час оновлення даних в пасивному режимі:")
        self.tabParameters.lb_tmWait = QLabel("Час переходу в пасивний режим:")

        self.tabParameters.spn_tmUpd = QSpinBox()
        self.tabParameters.spn_tmPasUpd = QSpinBox()
        self.tabParameters.spn_tmWait = QSpinBox()

        self.tabParameters.spn_tmUpd.setMaximum(8640000)
        self.tabParameters.spn_tmPasUpd.setMaximum(8640000)
        self.tabParameters.spn_tmWait.setMaximum(8640000)
        
        self.tabParameters.fbox.addRow(self.tabParameters.lb_tmUpd, self.tabParameters.spn_tmUpd)
        self.tabParameters.fbox.addRow(self.tabParameters.lb_tmPasUpd, self.tabParameters.spn_tmPasUpd)
        self.tabParameters.fbox.addRow(self.tabParameters.lb_tmWait, self.tabParameters.spn_tmWait)

        self.tabParameters.mainLayout.addLayout(self.tabParameters.fbox)
        self.tabParameters.mainLayout.addWidget(QLabel())


        self.tabParameters.groupBox = QGroupBox('Контроль параметрів середовища')
        self.tabParameters.gridBox = QGridLayout()

        self.tabParameters.chk_temperature = QCheckBox("Температура, °C:")
        self.tabParameters.chk_pressure = QCheckBox("Тиск, mmHg:")
        self.tabParameters.chk_humidity = QCheckBox("Вологість повітря:")
        self.tabParameters.chk_soilMoisture = QCheckBox("Вологість ґрунту:")

        self.tabParameters.spn_minTemp = QSpinBox()
        self.tabParameters.spn_maxTemp = QSpinBox()

        self.tabParameters.spn_minPress = QSpinBox()
        self.tabParameters.spn_maxPress = QSpinBox()
        
        self.tabParameters.spn_minHum = QSpinBox()
        self.tabParameters.spn_maxHum = QSpinBox()
        
        self.tabParameters.spn_minSoilMoist = QSpinBox()
        self.tabParameters.spn_maxSoilMoist = QSpinBox()

        self.tabParameters.spn_minTemp.setMinimum(-35)
        self.tabParameters.spn_minTemp.setMaximum(80)

        self.tabParameters.spn_maxTemp.setMinimum(-35)
        self.tabParameters.spn_maxTemp.setMaximum(80)

        self.tabParameters.spn_minPress.setMinimum(240)
        self.tabParameters.spn_minPress.setMaximum(810)

        self.tabParameters.spn_maxPress.setMinimum(240)
        self.tabParameters.spn_maxPress.setMaximum(810)

        self.tabParameters.spn_minHum.setMinimum(0)
        self.tabParameters.spn_minHum.setMaximum(100)

        self.tabParameters.spn_maxHum.setMinimum(0)
        self.tabParameters.spn_maxHum.setMaximum(100)

        self.tabParameters.spn_minSoilMoist.setMinimum(0)
        self.tabParameters.spn_minSoilMoist.setMaximum(300)

        self.tabParameters.spn_maxSoilMoist.setMinimum(0)
        self.tabParameters.spn_maxSoilMoist.setMaximum(300)

        self.tabParameters.lb_w = QLabel(' —')
        self.tabParameters.lb_w.setMaximumWidth(20)

        self.tabParameters.gridBox.addWidget(self.tabParameters.chk_temperature, 0, 0)
        self.tabParameters.gridBox.addWidget(self.tabParameters.spn_minTemp, 0, 1)
        self.tabParameters.gridBox.addWidget(self.tabParameters.lb_w, 0, 2)
        self.tabParameters.gridBox.addWidget(self.tabParameters.spn_maxTemp, 0, 3)

        self.tabParameters.gridBox.addWidget(self.tabParameters.chk_pressure, 1, 0)
        self.tabParameters.gridBox.addWidget(self.tabParameters.spn_minPress, 1, 1)
        self.tabParameters.gridBox.addWidget(QLabel(' —'), 1, 2)
        self.tabParameters.gridBox.addWidget(self.tabParameters.spn_maxPress, 1, 3)

        self.tabParameters.gridBox.addWidget(self.tabParameters.chk_humidity, 2, 0)
        self.tabParameters.gridBox.addWidget(self.tabParameters.spn_minHum, 2, 1)
        self.tabParameters.gridBox.addWidget(QLabel(' —'), 2, 2)
        self.tabParameters.gridBox.addWidget(self.tabParameters.spn_maxHum, 2, 3)
        
        self.tabParameters.gridBox.addWidget(self.tabParameters.chk_soilMoisture, 3, 0)
        self.tabParameters.gridBox.addWidget(self.tabParameters.spn_minSoilMoist, 3, 1)
        self.tabParameters.gridBox.addWidget(QLabel(' —'), 3, 2)
        self.tabParameters.gridBox.addWidget(self.tabParameters.spn_maxSoilMoist, 3, 3)

        self.tabParameters.groupBox.setLayout(self.tabParameters.gridBox)
        self.tabParameters.mainLayout.addWidget(self.tabParameters.groupBox)


        self.tabParameters.groupBox2 = QGroupBox('Одиниці вимірювання')
        self.tabParameters.hbox_in = QHBoxLayout()

        self.tabParameters.cmb_temp = QComboBox()
        self.tabParameters.cmb_press = QComboBox()

        self.tabParameters.cmb_temp.addItems(['°C', '°F'])
        self.tabParameters.cmb_press.addItems(['Pa', 'mmHg', 'inHg'])

        self.tabParameters.hbox_in.addWidget(self.tabParameters.cmb_temp)
        self.tabParameters.hbox_in.addWidget(self.tabParameters.cmb_press)
        self.tabParameters.hbox_in.addStretch(1)

        self.tabParameters.groupBox2.setLayout(self.tabParameters.hbox_in)

        self.tabParameters.hbox_out = QHBoxLayout()

        self.tabParameters.hbox_out.addWidget(self.tabParameters.groupBox2)
        self.tabParameters.hbox_out.addStretch(1)

        self.tabParameters.mainLayout.addLayout(self.tabParameters.hbox_out)


        self.tabParameters.hbox_bottom = QHBoxLayout()

        self.tabParameters.btn_apply = QPushButton('Застосувати')
        self.tabParameters.btn_apply.clicked.connect(self.setConfig)

        self.tabParameters.hbox_bottom.addStretch(1)
        self.tabParameters.hbox_bottom.addWidget(self.tabParameters.btn_apply)

        self.tabParameters.mainLayout.addLayout(self.tabParameters.hbox_bottom)

        self.tabParameters.mainLayout.addStretch(1)

        self.tabParameters.setLayout(self.tabParameters.mainLayout)

        ####################

        ##### tab data #####

        self.tabData.mainLayout = QVBoxLayout()

        self.tabData.lb_date = QLabel("Дата оновлення:")
        self.tabData.lb_time = QLabel("Час оновлення:")
        self.tabData.lb_temperature = QLabel("Температура повітря:")
        self.tabData.lb_pressure = QLabel("Атмосферний тиск:")
        self.tabData.lb_humidity = QLabel("Вологість повітря:")
        self.tabData.lb_soilMoisture = QLabel("Вологість ґрунту:")

        self.tabData.mainLayout.addWidget(self.tabData.lb_date)
        self.tabData.mainLayout.addWidget(self.tabData.lb_time)
        self.tabData.mainLayout.addWidget(QLabel())
        self.tabData.mainLayout.addWidget(self.tabData.lb_temperature)
        self.tabData.mainLayout.addWidget(self.tabData.lb_pressure)
        self.tabData.mainLayout.addWidget(self.tabData.lb_humidity)
        self.tabData.mainLayout.addWidget(self.tabData.lb_soilMoisture)
        self.tabData.mainLayout.addStretch(1)

        self.tabData.setLayout(self.tabData.mainLayout)

        ####################

    def checkPorts(self):
        ports = QSerialPortInfo.availablePorts()
        portList = [port.portName() for port in ports]
        
        self.tabPort.comboCom.clear()
        self.tabPort.comboCom.addItems(portList)
    
    def openCom(self):
        port = self.tabPort.comboCom.currentText()
        self.serial.setPortName(port)
        self.serial.open(QIODevice.ReadWrite)
        self.tabPort.groupBox.setTitle("Відкрито порт " + port)
    
    def closeCom(self):
        if self.serial.isOpen():
            self.serial.close()
            self.tabPort.groupBox.setTitle("Відкрийте порт, по якому підключений пристрій")

    def onRead(self):
        pack = str(self.serial.readAll()).strip().replace('*', '°')[2:-5]
        print(pack)
        parsePack = pack.split(',')
        tm = datetime.datetime.now()

        if len(parsePack) == 4:
            self.tabData.lb_date.setText("Дата оновлення:     \t" + str(tm)[0:10])
            self.tabData.lb_time.setText("Час оновлення:      \t" + str(tm)[11:19])
            self.tabData.lb_temperature.setText("Температура повітря:\t" + parsePack[0])
            self.tabData.lb_pressure.setText("Атмосферний тиск:   \t" + parsePack[1])
            self.tabData.lb_humidity.setText("Вологість повітря:  \t" + parsePack[2] + ' %')
            self.tabData.lb_soilMoisture.setText("Вологість ґрунту:   \t" + parsePack[3] + ' %')

    def setConfig(self):
        if not self.serial.isOpen():
            self.show_warn_messagebox()
            return False

        package = "" + str(self.tabParameters.spn_tmUpd.value())
        package += "," + str(self.tabParameters.spn_tmPasUpd.value())
        package += "," + str(self.tabParameters.spn_tmWait.value())

        package += "," + str(int(self.tabParameters.chk_temperature.isChecked()))
        package += "," + str(self.tabParameters.spn_minTemp.value())
        package += "," + str(self.tabParameters.spn_maxTemp.value())

        package += "," + str(int(self.tabParameters.chk_pressure.isChecked()))
        package += "," + str(self.tabParameters.spn_minPress.value())
        package += "," + str(self.tabParameters.spn_maxPress.value())

        package += "," + str(int(self.tabParameters.chk_humidity.isChecked()))
        package += "," + str(self.tabParameters.spn_minHum.value())
        package += "," + str(self.tabParameters.spn_maxHum.value())

        package += "," + str(int(self.tabParameters.chk_soilMoisture.isChecked()))
        package += "," + str(self.tabParameters.spn_minSoilMoist.value())
        package += "," + str(self.tabParameters.spn_maxSoilMoist.value())

        package += "," + str(int(self.tabParameters.cmb_temp.currentIndex()))
        package += "," + str(self.tabParameters.cmb_press.currentIndex())

        self.serial.write(package.encode())

        self.show_info_messagebox()

    def show_info_messagebox(self):
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Information)

        msg.setWindowTitle(" ")
        msg.setText("Параметри передано на вказаний пристрій")
        
        msg.setStandardButtons(QMessageBox.Cancel)
        
        msg.exec_()
    
    def show_warn_messagebox(self):
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Warning)

        msg.setWindowTitle(" ")
        msg.setText("З'єднання не встановлено")
        
        msg.setStandardButtons(QMessageBox.Cancel)
        
        msg.exec_()



if __name__ == "__main__":
    App = QApplication(sys.argv)
    window = Window()

    window.show()
    sys.exit(App.exec_())
