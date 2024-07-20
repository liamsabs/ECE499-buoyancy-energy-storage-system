from control_panel import *
import _thread, time, spidev

recieved_data_LIFO = []

spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 100000

control = cntrl_pnl(spi)

def spiRecieve17Bytes_forever():
    global recieved_data_LIFO
    while(True):
        recieved_data = spi.xfer2([0x04] + [0x00] * 5)
        if recieved_data != [0 for i in range(6)]:
            recieved_data_LIFO.append(recieved_data)
            control.update_display(recieved_data)
        time.sleep(0.05)

_thread.start_new_thread(spiRecieve17Bytes_forever, ())

control.start_cntrl_pnl()
