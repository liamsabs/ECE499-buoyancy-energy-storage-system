from control_panel import *
import _thread, time, spidev
import RPi.GPIO as GPIO

DSPIC_TRANSMIT_READY = 25 # pin 25 will go high when the MC is ready to communicate

GPIO.setmode(GPIO.BCM) # set pin notation to match datasheet, not pin number
GPIO.setup(DSPIC_TRANSMIT_READY, GPIO.IN) # set up data transmot ready pin

spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 10000

control = cntrl_pnl(spi)

def spi_comms_loop():

    while(True):

        # check if the MC is ready to communicate
        if (GPIO.input(DSPIC_TRANSMIT_READY) == GPIO.HIGH):

            # default command is a data request
            command = [DATA_RQ_STATE] + [0x00] * RECIEVED_DATA_LENGTH

            # replacve with command from control panel if available
            if (control.new_command_available()):
                command = control.get_last_command()

            # send command
            recieved_data = spi.xfer2(command)

            # deal with recieved data
            if recieved_data != [0 for i in range(RECIEVED_DATA_LENGTH + 1)]:
                control.update_display(recieved_data)

            #time.sleep(0.5)

_thread.start_new_thread(spi_comms_loop, ())

control.start_cntrl_pnl()
