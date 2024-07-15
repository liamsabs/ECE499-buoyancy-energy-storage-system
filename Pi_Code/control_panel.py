from tkinter import *
from tkinter import ttk

class cntrl_pnl:

    def __init__(self):

        self.window_width = 775

        self.root = Tk()
        self.root.title('CONTROL PANEL')
        self.root.config(padx=10, pady=10, background='#ADD8E6')

        self.controls_fr = ttk.Frame(self.root, padding=(0, 0, 0, 0), relief='')
        self.display_fr = ttk.Frame(self.root, padding=(12, 12, 12, 12), relief='')

        # CONTROL BUTTONS

        self.stop_btn = ttk.Button(self.controls_fr, text='STOP', command=self.stop_btn_handler)
        self.generate_btn = ttk.Button(self.controls_fr, text='GENERATE', command=self.generate_btn_handler)
        self.store_btn = ttk.Button(self.controls_fr, text='STORE', command=self.store_btn_handler)

        # DISPLAY

        self.num_disp_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', width=1000, height=1000)

        self.screen_reset_button = ttk.Button(self.num_disp_fr, text='reset output', command=self.update_display)
        self.num_disp_obj = Text(self.num_disp_fr, wrap='word', padx=10, pady=10, width=30)

        self.power_generating_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.power_generating_label = ttk.Label(self.power_generating_fr, padding=(0, 0, 0, 0), text="GENERATING (W):", justify='left', anchor='w', relief='')
        self.power_generating_disp_obj = Text(self.power_generating_fr, wrap='word', padx=10, pady=10, width=30, height=20)

        self.power_using_fr = ttk.Frame(self.display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
        self.power_using_label = ttk.Label(self.power_using_fr, padding=(0, 0, 0, 0), text="USING (W):", justify='left', anchor='w', relief='')
        self.power_using_disp_obj = Text(self.power_using_fr, wrap='word', padx=10, pady=10, width=30, height=20)

        # GRID

        self.root.minsize(width=self.window_width, height=300)
        #self.root.maxsize(width=2000, height=5000)
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(2, weight=1)

        self.controls_fr.grid(column=0, row=0, sticky=(W))
        self.stop_btn.grid(column=0, row=0, sticky=(W))
        self.generate_btn.grid(column=1, row=0, sticky=(W))
        self.store_btn.grid(column=2, row=0, sticky=(W))

        self.display_fr.grid(column=0, row=2, sticky=(W, N, E, S))
        self.display_fr.columnconfigure(0, weight=1)
        self.display_fr.rowconfigure(0, weight=1)
        self.display_fr.rowconfigure(1, weight=1)

        self.num_disp_fr.grid(column=0, row=0, sticky=(W, N, E, S), rowspan=2)
        self.screen_reset_button.grid(column=0, row=0, sticky=(W))
        self.num_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
        self.num_disp_fr.rowconfigure(1, weight=1)

        self.power_generating_fr.grid(column=1, row=0, sticky=(E))
        self.power_generating_label.grid(column=0, row=0, sticky=(W))
        self.power_generating_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
        self.power_generating_fr.rowconfigure(1, weight=1)

        self.power_using_fr.grid(column=1, row=1, sticky=(E))
        self.power_using_label.grid(column=0, row=0, sticky=(W))
        self.power_using_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
        self.power_using_fr.rowconfigure(1, weight=1)

        self.num_disp_fr.columnconfigure(0, weight=1)
        self.num_disp_fr.rowconfigure(1, weight=1)

        # START WINDOW

        self.root.update()
        self.root.after_idle(self.update_display)
        self.root.mainloop()

    def update_display(self, *args):
        self.num_disp_obj.delete(1.0, END)
        self.num_disp_obj.insert(1.0, "Group 21 - Bouyancy Energy Storage System (BESS)")

    def stop_btn_handler():
        print("handling stop button")

    def generate_btn_handler():
        print("handling generate button")

    def store_btn_handler():
        print("handling store button")
