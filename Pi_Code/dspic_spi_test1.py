# Computer = Rpi3B+
# Linux    = $ hostnamectl = raspberrypi Raspbian GNU/Linux 9 (stretch) Linux
# Python   = written for 3.5.3 Jan 19 2017, should work for any

# Test 1   - Send
# Setup    - Connect MOSI to scope to check output.
# Function - Repeat sending a byte, pause after each byte.

from time import sleep
import spidev

spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 100000

def spiSendOneByteRecieveSeventeenBytes(spiPort, sendData):
    sendDataArray = [sendData]
    recievedData = spi.xfer2(sendDataArray + [0x00] * 16)
    return recievedData

while (1):
    response = spiSendOneByteRecieveSeventeenBytes(0, 0x55)
    print("Recieved: " + str(response))
