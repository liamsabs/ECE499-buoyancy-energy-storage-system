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

recieved_data_FIFO = []

spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 100000

def spiRecieve17Bytes_forever():
    while(True):
        recieved_data = spi.xfer2([0x00] * 17)
        if recieved_data_FIFO != [0 for i in range(17)]:
            recieved_data_FIFO.append(recieved_data)

_thread.start_new_thread(spiRecieve17Bytes_forever())

#######################################################################################################################
################################################ DISPLAY RECIEVED DATA ################################################
#######################################################################################################################

# this pops the first thing in the FIFO queue and prints it to the screen.
# eventually it will print it on the screen of the dashboard

while(True):
    if len(recieved_data_FIFO) > 0:
        first_in_queue = recieved_data_FIFO.pop(0)
        print(first_in_queue)
    else:
        print("queue of recieved data is empty")
    time.sleep(1)
