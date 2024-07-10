from tkinter import *
from tkinter import ttk

window_width = 775

root = Tk()
root.title('CONTROL PANEL')
root.config(padx=10, pady=10, background='#ADD8E6')

controls_fr = ttk.Frame(root, padding=(0, 0, 0, 0), relief='')
display_fr = ttk.Frame(root, padding=(12, 12, 12, 12), relief='')

def update_display(*args):
    num_disp_obj.delete(1.0, END)
    num_disp_obj.insert(1.0, "Group 21 - Bouyancy Energy Storage System (BESS)")

def stop_btn_handler():
    print("handling stop button")

def generate_btn_handler():
    print("handling generate button")

def store_btn_handler():
    print("handling store button")

# CONTROL BUTTONS

stop_btn = ttk.Button(controls_fr, text='STOP', command=stop_btn_handler)
generate_btn = ttk.Button(controls_fr, text='GENERATE', command=generate_btn_handler)
store_btn = ttk.Button(controls_fr, text='STORE', command=store_btn_handler)

# DISPLAY

num_disp_fr = ttk.Frame(display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', width=1000, height=1000)

screen_reset_button = ttk.Button(num_disp_fr, text='reset output', command=update_display)
num_disp_obj = Text(num_disp_fr, wrap='word', padx=10, pady=10, width=30)

power_generating_fr = ttk.Frame(display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
power_generating_label = ttk.Label(power_generating_fr, padding=(0, 0, 0, 0), text="GENERATING (W):", justify='left', anchor='w', relief='')
power_generating_disp_obj = Text(power_generating_fr, wrap='word', padx=10, pady=10, width=30, height=20)

power_using_fr = ttk.Frame(display_fr, padding=(8, 8, 8, 8), borderwidth=2, relief='sunken', height=100)
power_using_label = ttk.Label(power_using_fr, padding=(0, 0, 0, 0), text="USING (W):", justify='left', anchor='w', relief='')
power_using_disp_obj = Text(power_using_fr, wrap='word', padx=10, pady=10, width=30, height=20)

# GRID

root.minsize(width=window_width, height=300)
#root.maxsize(width=2000, height=5000)
root.columnconfigure(0, weight=1)
root.rowconfigure(2, weight=1)

controls_fr.grid(column=0, row=0, sticky=(W))
stop_btn.grid(column=0, row=0, sticky=(W))
generate_btn.grid(column=1, row=0, sticky=(W))
store_btn.grid(column=2, row=0, sticky=(W))

display_fr.grid(column=0, row=2, sticky=(W, N, E, S))
display_fr.columnconfigure(0, weight=1)
display_fr.rowconfigure(0, weight=1)
display_fr.rowconfigure(1, weight=1)

num_disp_fr.grid(column=0, row=0, sticky=(W, N, E, S), rowspan=2)
screen_reset_button.grid(column=0, row=0, sticky=(W))
num_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
num_disp_fr.rowconfigure(1, weight=1)

power_generating_fr.grid(column=1, row=0, sticky=(E))
power_generating_label.grid(column=0, row=0, sticky=(W))
power_generating_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
power_generating_fr.rowconfigure(1, weight=1)

power_using_fr.grid(column=1, row=1, sticky=(E))
power_using_label.grid(column=0, row=0, sticky=(W))
power_using_disp_obj.grid(column=0, row=1, sticky=(W, N, E, S))
power_using_fr.rowconfigure(1, weight=1)

num_disp_fr.columnconfigure(0, weight=1)
num_disp_fr.rowconfigure(1, weight=1)

# START WINDOW

root.update()
root.after_idle(update_display)
root.mainloop()
