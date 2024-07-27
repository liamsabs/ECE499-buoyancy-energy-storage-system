from tkinter import *
from tkinter import ttk

RECIEVED_DATA_LENGTH = 10

STORING_STATE = 0x01
GENERATE_STATE = 0x02
PAUSE_STATE = 0x03
DATA_RQ_COMMAND = 0x04
IDLE_STATE = 0x05
STORED_STATE = 0x06

VOLTAGE_ADC_RATIO = 0459.596
CURRENT_ADC_RATIO = 1310.721

# computes two's complement value of "value" being an int of "num_bits" bit length
def twos_complement(value, num_bits):
    if (value & (1 << (num_bits - 1))) != 0: # if sign bit is set, compute negative value
        value = value - (1 << num_bits)
    return value

class BESS_control_panel:

    def __init__(self):

        self.window_width = 775

        self.root = Tk()
        self.root.title('Group 21 - Bouyancy Energy Storage System (BESS) - CONTROL PANEL')
        self.root.config(padx=10, pady=10, background='#ADD8E6')

        # buffer to store the SPI message for an outgoing command from the dashboard
        self.spi_rq_buffer = []
        self.spi_rq_buffer_flag = False # this flag is True if a new command is ready to be sent

        # stores the system state
        self.state = "await"
        self.prev_state = self.state
        self.is_load_on = IntVar()
        self.load_btn_update_flag = True

        self.controls_fr = ttk.Frame(self.root, padding=(0, 0, 0, 0), relief='')
        self.display_fr = ttk.Frame(self.root, padding=(12, 12, 12, 12), relief='')

        # CONTROL BUTTONS

        self.pause_btn = ttk.Button(self.controls_fr, text='PAUSE', command=self.pause_btn_handler)
        self.generate_btn = ttk.Button(self.controls_fr, text='GENERATE', command=self.generate_btn_handler)
        self.store_btn = ttk.Button(self.controls_fr, text='STORE', command=self.store_btn_handler)

        self.load_btn_fr = ttk.Frame(self.controls_fr, padding=(0, 0, 0, 0), relief='')
        self.load_btn_label = ttk.Label(self.load_btn_fr, padding=(0, 0, 0, 0), text="LOAD ENABLED:", justify='left', anchor='w', relief='')
        self.load_btn = ttk.Checkbutton(self.load_btn_fr, text="", variable=self.is_load_on, onvalue=1, offvalue=0, command=self.load_btn_handler)

        # DISPLAY

        self.data_disp_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', width=1000, height=1000)

        self.debug_clear_btn = ttk.Button(self.data_disp_fr, text='reset output', command=self.begin)
        self.debug_disp_obj = Text(self.data_disp_fr, wrap='word', padx=10, pady=10, width=30)

        self.widget_disp_fr = ttk.Frame(self.display_fr, padding=(0, 0, 0, 0), borderwidth=0, relief="flat")

        widget_disp_height = 1
        widget_disp_width = 20
        widget_disp_pad = 5

        self.V_bus_fr = ttk.Frame(self.widget_disp_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.V_bus_label = ttk.Label(self.V_bus_fr, padding=(0, 0, 0, 0), text="BUS VOLTAGE (V):", justify='left', anchor='w', relief='')
        self.V_bus_display = Text(self.V_bus_fr, wrap='word', padx=widget_disp_pad, pady=widget_disp_pad, width=widget_disp_width, height=widget_disp_height)

        self.W_bus_fr = ttk.Frame(self.widget_disp_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.W_bus_label = ttk.Label(self.W_bus_fr, padding=(0, 0, 0, 0), text="MOTOR POWER (W):", justify='left', anchor='w', relief='')
        self.W_bus_display = Text(self.W_bus_fr, wrap='word', padx=widget_disp_pad, pady=widget_disp_pad, width=widget_disp_width, height=widget_disp_height)

        self.A_bus_fr = ttk.Frame(self.widget_disp_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.A_bus_label = ttk.Label(self.A_bus_fr, padding=(0, 0, 0, 0), text="MOTOR CURRENT (A):", justify='left', anchor='w', relief='')
        self.A_bus_display = Text(self.A_bus_fr, wrap='word', padx=widget_disp_pad, pady=widget_disp_pad, width=widget_disp_width, height=widget_disp_height)

        self.W_bat_fr = ttk.Frame(self.widget_disp_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.W_bat_label = ttk.Label(self.W_bat_fr, padding=(0, 0, 0, 0), text="BATTERY POWER (W):", justify='left', anchor='w', relief='')
        self.W_bat_display = Text(self.W_bat_fr, wrap='word', padx=widget_disp_pad, pady=widget_disp_pad, width=widget_disp_width, height=widget_disp_height)

        self.A_bat_fr = ttk.Frame(self.widget_disp_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.A_bat_label = ttk.Label(self.A_bat_fr, padding=(0, 0, 0, 0), text="BATTERY CURRENT (A):", justify='left', anchor='w', relief='')
        self.A_bat_display = Text(self.A_bat_fr, wrap='word', padx=widget_disp_pad, pady=widget_disp_pad, width=widget_disp_width, height=widget_disp_height)

        self.state_fr = ttk.Frame(self.widget_disp_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.state_label = ttk.Label(self.state_fr, padding=(0, 0, 0, 0), text="STATE:", justify='left', anchor='w', relief='')
        self.state_display = Text(self.state_fr, wrap='word', padx=widget_disp_pad, pady=widget_disp_pad, width=widget_disp_width, height=widget_disp_height)

        self.depth_fr = ttk.Frame(self.widget_disp_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.depth_label = ttk.Label(self.depth_fr, padding=(0, 0, 0, 0), text="DEPTH (%):", justify='left', anchor='w', relief='')
        self.depth_display = Text(self.depth_fr, wrap='word', padx=widget_disp_pad, pady=widget_disp_pad, width=widget_disp_width, height=widget_disp_height)

        self.speed_fr = ttk.Frame(self.widget_disp_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.speed_label = ttk.Label(self.speed_fr, padding=(0, 0, 0, 0), text="MOTOR SPEED (RPM):", justify='left', anchor='w', relief='')
        self.speed_display = Text(self.speed_fr, wrap='word', padx=widget_disp_pad, pady=widget_disp_pad, width=widget_disp_width, height=widget_disp_height)

        # GRID

        self.root.minsize(width=self.window_width, height=538)
        #self.root.maxsize(width=2000, height=5000)
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(1, weight=1)

        self.controls_fr.grid(column=0, row=0, sticky=(W))
        self.pause_btn.grid(column=0, row=0, sticky=(W))
        self.generate_btn.grid(column=1, row=0, sticky=(W))
        self.store_btn.grid(column=2, row=0, sticky=(W))

        self.load_btn_fr.grid(column=3, row=0, sticky=(E))
        self.load_btn_label.grid(column=0, row=0, sticky=(E))
        self.load_btn.grid(column=1, row=0, sticky=(E))
        self.controls_fr.columnconfigure(0, weight=1)

        self.display_fr.grid(column=0, row=1, sticky=(W, N, E, S))
        self.display_fr.columnconfigure(0, weight=1)
        self.display_fr.rowconfigure(0, weight=1)
        #self.display_fr.rowconfigure(1, weight=1)

        self.data_disp_fr.grid(column=0, row=0, sticky=(W, N, E, S)) #, rowspan=7)
        self.debug_clear_btn.grid(column=0, row=0, sticky=(W))
        self.debug_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
        #self.num_disp_fr.rowconfigure(1, weight=1)

        self.widget_disp_fr.grid(column=1, row=0, sticky=(W, N, E, S))

        self.V_bus_fr.grid(column=0, row=0, sticky=(W, N))
        self.V_bus_label.grid(column=0, row=0, sticky=(W))
        self.V_bus_display.grid(column=0, row=1, sticky=(W, N, E, S))
        #self.V_bus_fr.rowconfigure(1, weight=1)

        self.W_bus_fr.grid(column=0, row=1, sticky=(W, N))
        self.W_bus_label.grid(column=0, row=0, sticky=(W))
        self.W_bus_display.grid(column=0, row=1, sticky=(W, N, E, S))
        #self.W_bus_fr.rowconfigure(1, weight=1)

        self.A_bus_fr.grid(column=0, row=2, sticky=(W, N))
        self.A_bus_label.grid(column=0, row=0, sticky=(W))
        self.A_bus_display.grid(column=0, row=1, sticky=(W, N, E, S))
        #self.A_bus_fr.rowconfigure(1, weight=1)

        self.W_bat_fr.grid(column=0, row=3, sticky=(W, N))
        self.W_bat_label.grid(column=0, row=0, sticky=(W))
        self.W_bat_display.grid(column=0, row=1, sticky=(W, N, E, S))
        #self.W_bat_fr.rowconfigure(1, weight=1)

        self.A_bat_fr.grid(column=0, row=4, sticky=(W, N))
        self.A_bat_label.grid(column=0, row=0, sticky=(W))
        self.A_bat_display.grid(column=0, row=1, sticky=(W, N, E, S))
        #self.A_bat_fr.rowconfigure(1, weight=1)

        self.state_fr.grid(column=0, row=5, sticky=(W, N))
        self.state_label.grid(column=0, row=0, sticky=(W))
        self.state_display.grid(column=0, row=1, sticky=(W, N, E, S))
        #self.state_fr.rowconfigure(1, weight=1)

        self.depth_fr.grid(column=0, row=6, sticky=(W, N))
        self.depth_label.grid(column=0, row=0, sticky=(W))
        self.depth_display.grid(column=0, row=1, sticky=(W, N, E, S))
        #self.depth_fr.rowconfigure(1, weight=1)

        self.speed_fr.grid(column=0, row=7, sticky=(W, N))
        self.speed_label.grid(column=0, row=0, sticky=(W))
        self.speed_display.grid(column=0, row=1, sticky=(W, N, E, S))
        #self.speed_fr.rowconfigure(1, weight=1)

        self.data_disp_fr.columnconfigure(0, weight=1)
        self.data_disp_fr.rowconfigure(1, weight=1)

    # START WINDOW
    def begin(self):
        self.root.update()
        #self.root.after_idle(self.update_display)
        self.debug_disp_obj.delete(1.0, END)
        self.debug_disp_obj.insert(1.0, "Group 21 - Bouyancy Energy Storage System (BESS)\n")
        self.root.mainloop()

    def create_command_request(self, value):
        self.spi_rq_buffer = [value] + [0x00] * RECIEVED_DATA_LENGTH
        self.spi_rq_buffer_flag = True

    def is_new_command_available(self):
        return self.spi_rq_buffer_flag

    def is_load_enabled(self):
        return True if self.is_load_on.get() else False

    def is_new_load_command_available(self):
        return True if self.load_btn_update_flag else False

    def load_btn_update_handled(self):
        self.load_btn_update_flag = False

    def get_latest_command(self):
        returnValue = self.spi_rq_buffer
        self.spi_rq_buffer = []
        self.spi_rq_buffer_flag = False
        return returnValue

    def update_display(self, values, *args):

        # handle debug console
        display_str = "["
        for v in values:
            display_str += " " + "{:03d}".format(v)
        display_str += " ]\n"

        self.debug_disp_obj.insert("end", display_str)
        self.debug_disp_obj.see("end")

        # set up data for display
        bus_voltage = ( (values[1] << 8) | values[2] ) / VOLTAGE_ADC_RATIO

        motor_current = twos_complement( (values[3] << 8) | values[4] , 16 ) / CURRENT_ADC_RATIO
        battery_current = twos_complement( (values[5] << 8) | values[6] , 16 ) / CURRENT_ADC_RATIO
        motor_power = bus_voltage * motor_current
        battery_power = bus_voltage * battery_current

        self.state = "state error"
        state_value_recieved = values[7]
        if state_value_recieved == PAUSE_STATE:
            self.state = "paused"
        elif state_value_recieved == GENERATE_STATE:
            self.state = "generating"
            self.prev_state = self.state
        elif state_value_recieved == STORING_STATE:
            self.state = "storing"
            self.prev_state = self.state
        elif state_value_recieved == STORED_STATE:
            self.state = "stored"
        elif state_value_recieved == IDLE_STATE:
            self.state = "idle"
        elif state_value_recieved == DATA_RQ_COMMAND:
            pass
        else:
            self.state = "state error"

        depth = values[8]
        speed_rpm = twos_complement( (values[9] << 8) | values[10] , 16 )

        # display values to designated text boxes
        self.V_bus_display.delete(1.0, END)
        self.W_bus_display.delete(1.0, END)
        self.A_bus_display.delete(1.0, END)
        self.W_bat_display.delete(1.0, END)
        self.A_bat_display.delete(1.0, END)
        self.state_display.delete(1.0, END)
        self.depth_display.delete(1.0, END)
        self.speed_display.delete(1.0, END)

        self.V_bus_display.insert("end", "{:2.2f}".format(bus_voltage))
        self.W_bus_display.insert("end", "{:2.2f}".format(motor_power))
        self.A_bus_display.insert("end", "{:2.2f}".format(motor_current))
        self.W_bat_display.insert("end", "{:2.2f}".format(battery_power))
        self.A_bat_display.insert("end", "{:2.2f}".format(battery_current))
        if self.state == "paused":
            self.state_display.insert("end", self.prev_state + "-")
        self.state_display.insert("end", str(self.state))
        self.depth_display.insert("end", str(depth))
        self.speed_display.insert("end", str(speed_rpm))

        #print("updated display")
        #self.root.after(1000, self.update_display)

    def pause_btn_handler(self):
        self.debug_disp_obj.insert("end", "pause button pressed\n")
        self.debug_disp_obj.see("end")
        self.create_command_request(PAUSE_STATE)

    def generate_btn_handler(self):
        self.debug_disp_obj.insert("end", "generate button pressed\n")
        self.debug_disp_obj.see("end")
        self.create_command_request(GENERATE_STATE)

    def store_btn_handler(self):
        self.debug_disp_obj.insert("end", "store button pressed\n")
        self.debug_disp_obj.see("end")
        self.create_command_request(STORING_STATE)

    def load_btn_handler(self):
        self.load_btn_update_flag = True
        if self.is_load_on.get():
            self.debug_disp_obj.insert("end", "load enabled\n")
        else:
            self.debug_disp_obj.insert("end", "load disabled\n")
        self.debug_disp_obj.see("end")

#cntrl = BESS_control_panel()
#cntrl.begin()
