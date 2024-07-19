from control_panel import *
import _thread, time, spidev

#######################################################################################################################
################################################# CONTROL PANEL CODE ##################################################
#######################################################################################################################

control = cntrl_pnl()
_thread.start_new_thread(control.start_cntrl_pnl())

#######################################################################################################################
################################################## SPI RECIEVER CODE ##################################################
#######################################################################################################################

recieved_data_LIFO = []

spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 100000

def spiRecieve17Bytes_forever():
    global recieved_data_LIFO
    while(True):
        recieved_data = spi.xfer2([0x04] + [0x00] * 5)      
        if recieved_data != [0 for i in range(6)]:
            recieved_data_LIFO.append(recieved_data)
        time.sleep(0.05)

_thread.start_new_thread(spiRecieve17Bytes_forever, ())

#######################################################################################################################
################################################ DISPLAY RECIEVED DATA ################################################
#######################################################################################################################

# this pops the first thing in the FIFO queue and prints it to the screen.
# eventually it will print it on the screen of the dashboard

while(True):
    if len(recieved_data_LIFO) > 0:
        last_in_queue = recieved_data_LIFO.pop(-1)
        print(last_in_queue)
        recieved_data_LIFO = []
    else:
        print("queue of recieved data is empty")
    time.sleep(1)

