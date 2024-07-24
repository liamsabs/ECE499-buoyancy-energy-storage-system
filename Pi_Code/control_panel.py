from tkinter import *
from tkinter import ttk

RECIEVED_DATA_LENGTH = 8

STORING_STATE = 0x01
GENERATE_STATE = 0x02
PAUSE_STATE = 0x03
DATA_RQ_COMMAND = 0x04
IDLE_STATE = 0x05
STORED_STATE = 0x06

VOLTAGE_ADC_RATIO = 459.596
CURRENT_ADC_RATIO = 054.067

class BESS_control_panel:

    def __init__(self):

        # buffer to store the SPI message for an outgoing command from the dashboard
        self.spi_rq_buffer = []
        self.spi_rq_buffer_flag = False # this flag is True if a new command is ready to be sent

        # stores the system state
        self.state = "await"
        self.prev_state = self.state

        self.window_width = 775

        self.root = Tk()
        self.root.title('CONTROL PANEL')
        self.root.config(padx=10, pady=10, background='#ADD8E6')

        self.controls_fr = ttk.Frame(self.root, padding=(0, 0, 0, 0), relief='')
        self.display_fr = ttk.Frame(self.root, padding=(12, 12, 12, 12), relief='')

        # CONTROL BUTTONS

        self.pause_btn = ttk.Button(self.controls_fr, text='PAUSE', command=self.pause_btn_handler)
        self.generate_btn = ttk.Button(self.controls_fr, text='GENERATE', command=self.generate_btn_handler)
        self.store_btn = ttk.Button(self.controls_fr, text='STORE', command=self.store_btn_handler)

        # DISPLAY

        widget_height = 1
        widget_width = 20

        self.num_disp_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', width=1000, height=1000)

        self.screen_reset_button = ttk.Button(self.num_disp_fr, text='reset output', command=self.begin)
        self.num_disp_obj = Text(self.num_disp_fr, wrap='word', padx=10, pady=10, width=30)

        self.W_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.W_label = ttk.Label(self.W_fr, padding=(0, 0, 0, 0), text="POWER (W):", justify='left', anchor='w', relief='')
        self.W_display = Text(self.W_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

        self.V_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.V_label = ttk.Label(self.V_fr, padding=(0, 0, 0, 0), text="VOLTAGE (V):", justify='left', anchor='w', relief='')
        self.V_display = Text(self.V_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

        self.A_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.A_label = ttk.Label(self.A_fr, padding=(0, 0, 0, 0), text="CURRENT (A):", justify='left', anchor='w', relief='')
        self.A_display = Text(self.A_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

        self.state_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.state_label = ttk.Label(self.state_fr, padding=(0, 0, 0, 0), text="STATE:", justify='left', anchor='w', relief='')
        self.state_display = Text(self.state_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

        self.depth_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.depth_label = ttk.Label(self.depth_fr, padding=(0, 0, 0, 0), text="DEPTH (%):", justify='left', anchor='w', relief='')
        self.depth_display = Text(self.depth_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

        self.speed_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.speed_label = ttk.Label(self.speed_fr, padding=(0, 0, 0, 0), text="SPEED (RPM):", justify='left', anchor='w', relief='')
        self.speed_display = Text(self.speed_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

        # GRID

        self.root.minsize(width=self.window_width, height=300)
        #self.root.maxsize(width=2000, height=5000)
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(2, weight=1)

        self.controls_fr.grid(column=0, row=0, sticky=(W))
        self.pause_btn.grid(column=0, row=0, sticky=(W))
        self.generate_btn.grid(column=1, row=0, sticky=(W))
        self.store_btn.grid(column=2, row=0, sticky=(W))

        self.display_fr.grid(column=0, row=2, sticky=(W, N, E, S))
        self.display_fr.columnconfigure(0, weight=1)
        self.display_fr.rowconfigure(0, weight=1)
        self.display_fr.rowconfigure(1, weight=1)

        self.num_disp_fr.grid(column=0, row=0, sticky=(W, N, E, S), rowspan=6)
        self.screen_reset_button.grid(column=0, row=0, sticky=(W))
        self.num_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
        self.num_disp_fr.rowconfigure(1, weight=1)

        self.W_fr.grid(column=1, row=0, sticky=(E))
        self.W_label.grid(column=0, row=0, sticky=(W))
        self.W_display.grid(column=0, row=1, sticky=(W, N, E, S))
        self.W_fr.rowconfigure(1, weight=1)

        self.V_fr.grid(column=1, row=1, sticky=(E))
        self.V_label.grid(column=0, row=0, sticky=(W))
        self.V_display.grid(column=0, row=1, sticky=(W, N, E, S))
        self.V_fr.rowconfigure(1, weight=1)

        self.A_fr.grid(column=1, row=2, sticky=(E))
        self.A_label.grid(column=0, row=0, sticky=(W))
        self.A_display.grid(column=0, row=1, sticky=(W, N, E, S))
        self.A_fr.rowconfigure(1, weight=1)

        self.state_fr.grid(column=1, row=3, sticky=(E))
        self.state_label.grid(column=0, row=0, sticky=(W))
        self.state_display.grid(column=0, row=1, sticky=(W, N, E, S))
        self.state_fr.rowconfigure(1, weight=1)

        self.depth_fr.grid(column=1, row=4, sticky=(E))
        self.depth_label.grid(column=0, row=0, sticky=(W))
        self.depth_display.grid(column=0, row=1, sticky=(W, N, E, S))
        self.depth_fr.rowconfigure(1, weight=1)

        self.speed_fr.grid(column=1, row=5, sticky=(E))
        self.speed_label.grid(column=0, row=0, sticky=(W))
        self.speed_display.grid(column=0, row=1, sticky=(W, N, E, S))
        self.speed_fr.rowconfigure(1, weight=1)

        self.num_disp_fr.columnconfigure(0, weight=1)
        self.num_disp_fr.rowconfigure(1, weight=1)

    # START WINDOW
    def begin(self):
        self.root.update()
        #self.root.after_idle(self.update_display)
        self.num_disp_obj.delete(1.0, END)
        self.num_disp_obj.insert(1.0, "Group 21 - Bouyancy Energy Storage System (BESS)\n")
        self.root.mainloop()

    def create_command_request(self, value):
        self.spi_rq_buffer = [value] + [0x00] * RECIEVED_DATA_LENGTH
        self.spi_rq_buffer_flag = True

    def is_new_command_available(self):
        return self.spi_rq_buffer_flag

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

        self.num_disp_obj.insert("end", display_str)
        self.num_disp_obj.see("end")

        # set up data for display
        voltage = ( (values[1] << 8) | values[2] ) / VOLTAGE_ADC_RATIO
        current = ( (values[3] << 8) | values[4] ) / CURRENT_ADC_RATIO
        depth = values[6]
        speed_rpm = (values[7] << 8) | values[8]
        power = voltage * current

        self.state = "state error"
        state_value_recieved = values[5]
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

        # display values to designated text boxes
        self.W_display.delete(1.0, END)
        self.V_display.delete(1.0, END)
        self.A_display.delete(1.0, END)
        self.state_display.delete(1.0, END)
        self.depth_display.delete(1.0, END)
        self.speed_display.delete(1.0, END)

        self.W_display.insert("end", "{:2.2f}".format(power))
        self.V_display.insert("end", "{:2.2f}".format(voltage))
        self.A_display.insert("end", "{:2.2f}".format(current))
        if self.state == "paused":
            self.state_display.insert("end", self.prev_state + "-")
        self.state_display.insert("end", str(self.state))
        self.depth_display.insert("end", str(depth))
        self.speed_display.insert("end", str(speed_rpm))

        #print("updated display")
        #self.root.after(1000, self.update_display)

    def pause_btn_handler(self):
        print("handling pause button")
        self.create_command_request(PAUSE_STATE)

    def generate_btn_handler(self):
        print("handling generate button")
        self.create_command_request(GENERATE_STATE)

    def store_btn_handler(self):
        print("handling store button")
        self.create_command_request(STORING_STATE)
