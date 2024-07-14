
# Computer = Rpi3B+
# Linux    = $ hostnamectl = raspberrypi Raspbian GNU/Linux 9 (stretch) Linux
# Python   = written for 3.5.3 Jan 19 2017, should work for any

# Test 1   - repeatSendByte() - repeatedly sends out single bytes via SPI.
# Setup    - Connect MOSI to scope to check output.
# Function - Repeat sending a byte, pause after each byte.

# Test 2   - loopBackTest()   - send and receive one byte via SPI.
# Setup    - Connect MOSI pin to MISO pin to form a loop.
# Function - Send one byte to MSOI and read it back from MISO.

from time import sleep
import spidev

spiPort0 = spidev.SpiDev()
spiPort0.open(0,0)
spiPort0.max_speed_hz = 100000

def spiSendRecvOneByte(spiPort, sendByte):
    sendByteArray = [sendByte]
    recvByteArray = spiPort.xfer(sendByteArray)
    return recvByteArray

def repeatSendOneByte(spiPort, sendByte, pauseTimeBetweenBytes, repeatCount):
    print('\nBegin repeatSendByte(),....')
    for i in range(repeatCount):
        spiSendRecvOneByte(spiPort, sendByte)
        sleep(pauseTimeBetweenBytes)
    print('End   repeatSendByte().')
    return

def loopBackOneByte(spiPort, sendByte):
    recvByteArray     = spiSendRecvOneByte(spiPort, sendByte)
    recvByte          = recvByteArray[0]
    print('\nBegin testLoopbackOneByte(),....')
    print('      sendByte  = ', hex(sendByte))
    print('      recvByte  = ', hex(recvByte))
    print('End   testLoopbackOneByte(),....')
    return

repeatSendOneByte(spiPort0, 0x5b, 0.0001, 20000000)
#loopBackOneByte(spiPort0, 0x5b)

'''
Sample output
Begin testLoopbackOneByte(),....
      sendByte  =  0x5b
      recvByte  =  0x5b
End   testLoopbackOneByte(),....
'''
