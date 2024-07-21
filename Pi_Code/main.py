from control_panel import *
import _thread, time, spidev
import RPi.GPIO as GPIO

MC_transmit_ready_pin = 25 # pin 25 will go high when the MC is ready to communicate

GPIO.setmode(GPIO.BCM) # set pin notation to match datasheet, not pin number
GPIO.setup(MC_transmit_ready_pin, GPIO.IN)

spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 10000

control = cntrl_pnl(spi)

def Pi_MC_comms_loop():

	while(True):

		# check if the MC is ready to communicate
		if (GPIO.input(MC_transmit_ready_pin) == GPIO.HIGH):

			# default command is a data request
			command = [0x04] + [0x00] * 5

			# replacve with command from control panel if available
			if (control.new_command_available()):
				command = control.get_last_command()

			# send command
			recieved_data = spi.xfer2(command)

			# deal with recieved data
			if recieved_data != [0 for i in range(6)]:
				control.update_display(recieved_data)

			#time.sleep(0.5)

_thread.start_new_thread(Pi_MC_comms_loop, ())

control.start_cntrl_pnl()
