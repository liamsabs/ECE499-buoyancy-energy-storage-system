from control_panel import *
import _thread, time, spidev
import RPi.GPIO as GPIO

# pin 25 will go high when the MC is ready to communicate
DSPIC_TRANSMIT_READY = 25

# set pin notation to match datasheet, not pin number
GPIO.setmode(GPIO.BCM)

# set up data transmit ready pin
GPIO.setup(DSPIC_TRANSMIT_READY, GPIO.IN)

# set up SPI on SPI 0
spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 10000

# initialize control panel
control_panel = BESS_control_panel()

# this function runs in a separate thread from the control panel
# it sends commands from the control panel and sends system data to the control panel
def spi_comms_loop():

    while(True):

        # check if the MC is ready to communicate
        if (GPIO.input(DSPIC_TRANSMIT_READY) == GPIO.HIGH):

            # default command is a data request
            command = [DATA_RQ_COMMAND] + [0x00] * RECIEVED_DATA_LENGTH

            # replacve with command from control panel if available
            if (control_panel.is_new_command_available()):
                command = control_panel.get_latest_command()

            # send command
            recieved_data = spi.xfer2(command)

            # manage recieved data
            if recieved_data != [0 for i in range(RECIEVED_DATA_LENGTH + 1)]:
                control_panel.update_display(recieved_data)

            #time.sleep(0.5)

# begin SPI communications with the DSPIC motor controller
_thread.start_new_thread(spi_comms_loop, ())

control_panel.begin()
