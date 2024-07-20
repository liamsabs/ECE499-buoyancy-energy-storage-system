from control_panel import *
import _thread, time, spidev
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(25, GPIO.IN)

recieved_data_LIFO = []

spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 100000

control = cntrl_pnl(spi)

def spiRecieve17Bytes_forever():
    global recieved_data_LIFO
    
    while(True):
        
        # check if the control panel has a command to send
        if (control.spi_request_buffer_flag):
            spi.xfer2(control.spi_request_buffer)
            control.spi_request_buffer = []
            control.spi_request_buffer_flag = False
            
        # check if the MC is ready to transmit
        if (GPIO.input(25) == GPIO.LOW):
            
            # check if the control panel has a command to send
            if (control.spi_request_buffer_flag):
                spi.xfer2(control.spi_request_buffer)
                control.spi_request_buffer = []
                control.spi_request_buffer_flag = False
            
            # initiate a data request
            recieved_data = spi.xfer2([0x04] + [0x00] * 5)
            if recieved_data != [0 for i in range(6)]:
                recieved_data_LIFO.append(recieved_data)
                control.update_display(recieved_data)
                
            #time.sleep(0.5)

_thread.start_new_thread(spiRecieve17Bytes_forever, ())

control.start_cntrl_pnl()
