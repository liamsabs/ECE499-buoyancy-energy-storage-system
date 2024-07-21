from tkinter import *
from tkinter import ttk

class cntrl_pnl:

	def __init__(self, spi):

		self.spi_request_buffer = []
		self.spi_request_buffer_flag = False

		self.spi = spi

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

		widget_height = 2
		widget_width = 5

		self.num_disp_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', width=1000, height=1000)

		self.screen_reset_button = ttk.Button(self.num_disp_fr, text='reset output', command=self.start_cntrl_pnl)
		self.num_disp_obj = Text(self.num_disp_fr, wrap='word', padx=10, pady=10, width=30)

		self.W_generating_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
		self.W_generating_label = ttk.Label(self.W_generating_fr, padding=(0, 0, 0, 0), text="GENERATING (W):", justify='left', anchor='w', relief='')
		self.W_generating_disp_obj = Text(self.W_generating_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

		self.W_using_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
		self.W_using_label = ttk.Label(self.W_using_fr, padding=(0, 0, 0, 0), text="USING (W):", justify='left', anchor='w', relief='')
		self.W_using_disp_obj = Text(self.W_using_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

		self.V_generating_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
		self.V_generating_label = ttk.Label(self.V_generating_fr, padding=(0, 0, 0, 0), text="GENERATING (V):", justify='left', anchor='w', relief='')
		self.V_generating_disp_obj = Text(self.V_generating_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

		self.V_using_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
		self.V_using_label = ttk.Label(self.V_using_fr, padding=(0, 0, 0, 0), text="USING (V):", justify='left', anchor='w', relief='')
		self.V_using_disp_obj = Text(self.V_using_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

		self.A_generating_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
		self.A_generating_label = ttk.Label(self.A_generating_fr, padding=(0, 0, 0, 0), text="GENERATING (A):", justify='left', anchor='w', relief='')
		self.A_generating_disp_obj = Text(self.A_generating_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

		self.A_using_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
		self.A_using_label = ttk.Label(self.A_using_fr, padding=(0, 0, 0, 0), text="USING (A):", justify='left', anchor='w', relief='')
		self.A_using_disp_obj = Text(self.A_using_fr, wrap='word', padx=10, pady=10, width=widget_width, height=widget_height)

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

		self.W_generating_fr.grid(column=1, row=0, sticky=(E))
		self.W_generating_label.grid(column=0, row=0, sticky=(W))
		self.W_generating_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
		self.W_generating_fr.rowconfigure(1, weight=1)

		self.W_using_fr.grid(column=1, row=1, sticky=(E))
		self.W_using_label.grid(column=0, row=0, sticky=(W))
		self.W_using_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
		self.W_using_fr.rowconfigure(1, weight=1)

		self.V_generating_fr.grid(column=1, row=2, sticky=(E))
		self.V_generating_label.grid(column=0, row=0, sticky=(W))
		self.V_generating_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
		self.V_generating_fr.rowconfigure(1, weight=1)

		self.V_using_fr.grid(column=1, row=3, sticky=(E))
		self.V_using_label.grid(column=0, row=0, sticky=(W))
		self.V_using_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
		self.V_using_fr.rowconfigure(1, weight=1)

		self.A_generating_fr.grid(column=1, row=4, sticky=(E))
		self.A_generating_label.grid(column=0, row=0, sticky=(W))
		self.A_generating_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
		self.A_generating_fr.rowconfigure(1, weight=1)

		self.A_using_fr.grid(column=1, row=5, sticky=(E))
		self.A_using_label.grid(column=0, row=0, sticky=(W))
		self.A_using_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
		self.A_using_fr.rowconfigure(1, weight=1)

		self.num_disp_fr.columnconfigure(0, weight=1)
		self.num_disp_fr.rowconfigure(1, weight=1)

	# START WINDOW
	def start_cntrl_pnl(self):
		self.root.update()
		#self.root.after_idle(self.update_display)
		self.num_disp_obj.delete(1.0, END)
		self.num_disp_obj.insert(1.0, "Group 21 - Bouyancy Energy Storage System (BESS)\n")
		self.root.mainloop()

	def set_spi_buffer(self, value):
		self.spi_request_buffer = value
		self.spi_request_buffer_flag = True

	def new_command_available(self):
		return self.spi_request_buffer_flag

	def get_last_command(self):
		returnValue = self.spi_request_buffer
		self.spi_request_buffer = []
		self.spi_request_buffer_flag = False
		return returnValue

	def update_display(self, values, *args):

		self.num_disp_obj.insert("end", str(values) + "\n")
		self.num_disp_obj.see("end")

		self.W_generating_disp_obj.delete(1.0, END)
		self.W_using_disp_obj.delete(1.0, END)
		self.V_generating_disp_obj.delete(1.0, END)
		self.V_using_disp_obj.delete(1.0, END)
		self.A_generating_disp_obj.delete(1.0, END)
		self.A_using_disp_obj.delete(1.0, END)

		self.W_generating_disp_obj.insert("end", str(values[0]))
		self.W_using_disp_obj.insert("end", str(values[1]))
		self.V_generating_disp_obj.insert("end", str(values[2]))
		self.V_using_disp_obj.insert("end", str(values[3]))
		self.A_generating_disp_obj.insert("end", str(values[4]))
		self.A_using_disp_obj.insert("end", str(values[5]))
		#print("updated display")
		#self.root.after(1000, self.update_display)

	def pause_btn_handler(self):
		print("handling pause button")
		self.set_spi_buffer([0x03] + [0x00] * 5)
		#self.spi.xfer2([0x03] + [0x00] * 5)

	def generate_btn_handler(self):
		print("handling generate button")
		self.set_spi_buffer([0x02] + [0x00] * 5)
		#self.spi.xfer2([0x02] + [0x00] * 5)

	def store_btn_handler(self):
		print("handling store button")
		self.set_spi_buffer([0x01] + [0x00] * 5)
		#self.spi.xfer2([0x01] + [0x00] * 5)
