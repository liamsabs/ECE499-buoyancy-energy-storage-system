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

def spiRecieve17Bytes_forever():

	while(True):

		# check if the MC is ready to communicate
		if (GPIO.input(MC_transmit_ready_pin) == GPIO.LOW):

			# send new command from control panel if available
			if (control.new_command_available()):
				spi.xfer2(control.get_last_command())

			# initiate a data request
			recieved_data = spi.xfer2([0x04] + [0x00] * 5)
			if recieved_data != [0 for i in range(6)]:
				control.update_display(recieved_data)

			#time.sleep(0.5)

_thread.start_new_thread(spiRecieve17Bytes_forever, ())

control.start_cntrl_pnl()
