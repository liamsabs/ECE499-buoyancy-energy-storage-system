
# run on a Raspberry Pi 3B+ (or a better RPi)

from control_panel import *
import _thread, time, spidev
import RPi.GPIO as GPIO

# pin 25 will go high when the MC is ready to communicate
DSPIC_TRANSMIT_READY = 25
LOAD_RELAY_PIN = 24

# set pin notation to match datasheet, not pin number
GPIO.setmode(GPIO.BCM)

# set up data transmit ready pin
GPIO.setup(DSPIC_TRANSMIT_READY, GPIO.IN)
GPIO.setup(LOAD_RELAY_PIN, GPIO.OUT)

# set up SPI on SPI 0
spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 10000

# initialize control panel
control_panel = BESS_control_panel()

# this function runs in a separate thread from the control panel
# it sends commands from the control panel and sends system data to the control panel
def control_loop():

    while True:

        if control_panel.is_new_load_command_available():                    # control load relay
            if control_panel.is_load_enabled():
                GPIO.output(LOAD_RELAY_PIN, GPIO.HIGH)
            else:
                GPIO.output(LOAD_RELAY_PIN, GPIO.LOW)
            control_panel.load_btn_update_handled()

        if (GPIO.input(DSPIC_TRANSMIT_READY) == GPIO.HIGH):                   # check if the MC is ready to communicate
            command = [DATA_RQ_COMMAND] + [0x00] * RECIEVED_DATA_LENGTH       # default command is a data request
            if (control_panel.is_new_command_available()):                    # replace with command from control panel if available
                command = control_panel.get_latest_command()
            recieved_data = spi.xfer2(command)                                # send command
            if recieved_data != [0 for i in range(RECIEVED_DATA_LENGTH + 1)]: # manage recieved data
                control_panel.update_display(recieved_data)

# begin SPI communications with the DSPIC motor controller
_thread.start_new_thread(control_loop, ())

control_panel.begin()
