import tkinter as tk
from tkinter import scrolledtext, ttk, messagebox
import serial
import threading
import time
from datetime import datetime
import tkintermapview
import os
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
from collections import deque
import matplotlib.dates as mdates
from matplotlib import style
from PIL import Image, ImageTk

# Set matplotlib style
style.use('dark_background')

# Adjust these constants for your setup
SERIAL_PORT = "COM5"  # Change as needed
BAUD_RATE = 115200
LOG_FILE = "cansat_log.txt"
CSV_FILE = "cansat_data.csv"
MAX_DATA_POINTS = 100  # Maximum number of data points to show on graphs

# Define image paths - make sure these files exist in your app directory
SPLASH_IMAGE_PATH = "cansat_splash.jpg"  # The elf on can image
SATELLITE_IMAGE_PATH = "satellite_face.jpg"  # The face with satellite dish image

# Define a vibrant color palette
COLORS = {
    'background': '#2E3440',
    'text': '#ECEFF4',
    'accent': '#88C0D0',
    'highlight': '#8FBCBB',
    'warning': '#EBCB8B',
    'error': '#BF616A',
    'success': '#A3BE8C',
    'graph_bg': '#3B4252',
    'graph_grid': '#4C566A',
    'button_bg': '#5E81AC',
    'button_active': '#81A1C1',
    'entry_bg': '#434C5E',
    'tab_bg': '#4C566A',
    'tab_active': '#81A1C1'
}

# Graph colors
GRAPH_COLORS = {
    'temperature': '#BF616A',  # Red
    'humidity': '#5E81AC',  # Blue
    'pressure': '#A3BE8C',  # Green
    'co2': '#EBCB8B',  # Yellow
    'altitude': '#B48EAD',  # Purple
    'acceleration': '#D08770',  # Orange
    'mq135': '#88C0D0',  # Cyan
    'mq4': '#81A1C1',  # Light blue
    'ldr': '#8FBCBB'  # Teal
}


class CustomStyle(ttk.Style):
    def __init__(self):
        super().__init__()
        self.theme_use('clam')  # Use clam as the base theme

        # Configure TNotebook (tabs)
        self.configure('TNotebook', background=COLORS['background'])
        self.configure('TNotebook.Tab', background=COLORS['tab_bg'],
                       foreground=COLORS['text'], padding=[10, 2])
        self.map('TNotebook.Tab', background=[('selected', COLORS['tab_active'])])

        # Configure TFrame
        self.configure('TFrame', background=COLORS['background'])

        # Configure TLabelframe
        self.configure('TLabelframe', background=COLORS['background'],
                       foreground=COLORS['text'])
        self.configure('TLabelframe.Label', background=COLORS['background'],
                       foreground=COLORS['text'])

        # Configure TButton
        self.configure('TButton', background=COLORS['button_bg'],
                       foreground=COLORS['text'], padding=[10, 5])
        self.map('TButton', background=[('active', COLORS['button_active'])])

        # Configure Scrollbar
        self.configure('TScrollbar', background=COLORS['background'],
                       troughcolor=COLORS['graph_bg'], borderwidth=0)

        # Special style for command buttons
        self.configure('Command.TButton', font=('Arial', 10, 'bold'))

        # Special style for labeled frames
        self.configure('Graph.TLabelframe', background=COLORS['graph_bg'], borderwidth=1)
        self.configure('Graph.TLabelframe.Label', background=COLORS['graph_bg'],
                       foreground=COLORS['text'], font=('Arial', 10, 'bold'))


class SplashScreen(tk.Toplevel):
    def __init__(self, parent):
        super().__init__(parent)
        self.parent = parent

        # Remove window decorations
        self.overrideredirect(True)

        # Set window position to center of screen
        self.geometry("600x600")
        self.center_window()

        # Make the window on top
        self.attributes("-topmost", True)

        # Set background image
        try:
            img = Image.open(SPLASH_IMAGE_PATH)
            img = img.resize((600, 600), Image.LANCZOS)
            self.bg_image = ImageTk.PhotoImage(img)
            self.bg_label = tk.Label(self, image=self.bg_image)
            self.bg_label.place(x=0, y=0, relwidth=1, relheight=1)
        except Exception as e:
            print(f"Error loading splash image: {e}")
            self.bg_label = tk.Label(self, text="CanSat Ground Station",
                                     font=("Arial", 24, "bold"),
                                     bg=COLORS['background'], fg=COLORS['accent'])
            self.bg_label.place(relx=0.5, rely=0.4, anchor='center')

        # Add a title
        title_label = tk.Label(self, text="CanSat Ground Station",
                               font=("Arial", 24, "bold"),
                               bg='black', fg=COLORS['accent'])
        title_label.place(relx=0.5, rely=0.1, anchor='center')

        # Add loading text
        self.loading_label = tk.Label(self, text="Loading...",
                                      font=("Arial", 14),
                                      bg='black', fg=COLORS['text'])
        self.loading_label.place(relx=0.5, rely=0.9, anchor='center')

        # Add start button - appears after short delay
        self.after(2000, self.show_start_button)

    def show_start_button(self):
        self.loading_label.destroy()
        start_button = tk.Button(self, text="Start Ground Station",
                                 font=("Arial", 14, "bold"),
                                 bg=COLORS['button_bg'], fg=COLORS['text'],
                                 activebackground=COLORS['button_active'],
                                 command=self.close_splash)
        start_button.place(relx=0.5, rely=0.9, anchor='center')

    def close_splash(self):
        self.destroy()
        self.parent.deiconify()  # Show main window

    def center_window(self):
        self.update_idletasks()
        width = self.winfo_width()
        height = self.winfo_height()
        x = (self.winfo_screenwidth() // 2) - (width // 2)
        y = (self.winfo_screenheight() // 2) - (height // 2)
        self.geometry(f'+{x}+{y}')


class CanSatGUI:
    def __init__(self, master):
        self.master = master
        master.title("CanSat Ground Station")
        master.geometry("1000x700")  # Larger initial window size
        master.configure(bg=COLORS['background'])

        # Hide main window initially for splash screen
        master.withdraw()

        # Apply custom style
        self.style = CustomStyle()

        # Debug mode flag
        self.debug_mode = True  # Set to True to enable debug messages

        # Add a title label
        title_label = tk.Label(master, text="CanSat Ground Station",
                               font=("Arial", 16, "bold"),
                               bg=COLORS['background'], fg=COLORS['accent'])
        title_label.pack(pady=(10, 5))

        # Status indicator
        self.status_frame = tk.Frame(master, bg=COLORS['background'])
        self.status_frame.pack(fill=tk.X, padx=10, pady=2)

        self.status_label = tk.Label(self.status_frame, text="Status: ",
                                     bg=COLORS['background'], fg=COLORS['text'])
        self.status_label.pack(side=tk.LEFT)

        self.status_indicator = tk.Label(self.status_frame, text="●",
                                         bg=COLORS['background'], fg=COLORS['error'])
        self.status_indicator.pack(side=tk.LEFT)

        self.status_text = tk.Label(self.status_frame, text="Disconnected",
                                    bg=COLORS['background'], fg=COLORS['text'])
        self.status_text.pack(side=tk.LEFT, padx=5)

        # Add debug indicator
        self.debug_indicator = tk.Label(self.status_frame, text="DEBUG MODE",
                                        bg=COLORS['background'], fg=COLORS['warning'])
        if self.debug_mode:
            self.debug_indicator.pack(side=tk.LEFT, padx=15)

        # Mission time tracking variables
        self.launch_time = None
        self.flight_mode_active = False
        self.first_data_time = None  # Store the time of first data point

        # Add mission timer display
        self.mission_timer = tk.Label(self.status_frame, text="T+00:00",
                                      font=("Arial", 12, "bold"),
                                      bg=COLORS['background'], fg=COLORS['accent'])
        self.mission_timer.pack(side=tk.RIGHT, padx=10)
        self.update_mission_timer()

        # GPS data storage for map
        self.gps_coordinates = []

        # Keep track of last values to only store new readings
        self.last_values = {
            'temperature': None,
            'humidity': None,
            'pressure': None,
            'co2': None,
            'altitude': None,
            'acceleration': None,
            'mq135': None,
            'mq4': None,
            'ldr': None
        }

        # Data storage for graphs - store [time_in_seconds, value] pairs
        self.data_time_stamps = deque(maxlen=MAX_DATA_POINTS)  # Main time reference
        self.data_temperature = deque(maxlen=MAX_DATA_POINTS)
        self.data_humidity = deque(maxlen=MAX_DATA_POINTS)
        self.data_pressure = deque(maxlen=MAX_DATA_POINTS)
        self.data_co2 = deque(maxlen=MAX_DATA_POINTS)
        self.data_altitude = deque(maxlen=MAX_DATA_POINTS)
        self.data_acceleration = deque(maxlen=MAX_DATA_POINTS)
        self.data_mq135 = deque(maxlen=MAX_DATA_POINTS)
        self.data_mq4 = deque(maxlen=MAX_DATA_POINTS)
        self.data_ldr = deque(maxlen=MAX_DATA_POINTS)

        # Track timestamps individually for each sensor
        self.time_temperature = deque(maxlen=MAX_DATA_POINTS)
        self.time_humidity = deque(maxlen=MAX_DATA_POINTS)
        self.time_pressure = deque(maxlen=MAX_DATA_POINTS)
        self.time_co2 = deque(maxlen=MAX_DATA_POINTS)
        self.time_altitude = deque(maxlen=MAX_DATA_POINTS)
        self.time_acceleration = deque(maxlen=MAX_DATA_POINTS)
        self.time_mq135 = deque(maxlen=MAX_DATA_POINTS)
        self.time_mq4 = deque(maxlen=MAX_DATA_POINTS)
        self.time_ldr = deque(maxlen=MAX_DATA_POINTS)

        # List to store all graph axes
        self.graph_axes = []

        # Frame for custom command entry
        command_frame = tk.Frame(master, bg=COLORS['background'])
        command_frame.pack(padx=10, pady=5, fill=tk.X)

        # Label for command entry
        cmd_label = tk.Label(command_frame, text="Command:",
                             bg=COLORS['background'], fg=COLORS['text'])
        cmd_label.pack(side=tk.LEFT, padx=(0, 5))

        self.command_entry = tk.Entry(command_frame, width=50,
                                      bg=COLORS['entry_bg'], fg=COLORS['text'],
                                      insertbackground=COLORS['text'],
                                      relief=tk.FLAT, bd=2)
        self.command_entry.pack(side=tk.LEFT, padx=5, pady=5, expand=True, fill=tk.X)

        self.send_button = tk.Button(command_frame, text="Send Custom",
                                     command=self.send_custom_command,
                                     bg=COLORS['button_bg'], fg=COLORS['text'],
                                     activebackground=COLORS['button_active'],
                                     activeforeground=COLORS['text'])
        self.send_button.pack(side=tk.LEFT, padx=5, pady=5)

        # Frame for command buttons
        button_frame = tk.Frame(master, bg=COLORS['background'])
        button_frame.pack(padx=10, pady=5, fill=tk.X)

        # Define the commands and corresponding functions with colors
        commands = [
            ("PRELAUNCH", self.send_prelaunch, COLORS['warning']),
            ("FLIGHT", self.send_flight, COLORS['success']),
            ("RECOVERY", self.send_recovery, COLORS['accent']),
            ("INIT_SCD40", self.send_init_scd40, COLORS['button_bg']),
            ("INIT_GPS", self.send_init_gps, COLORS['button_bg']),
            ("INIT_MQ", self.send_init_mq, COLORS['button_bg']),
            ("INIT_BOARD", self.send_init_board, COLORS['button_bg']),
        ]

        # Create buttons in a grid layout (3 columns) with custom colors
        for i, (label, func, color) in enumerate(commands):
            btn = tk.Button(button_frame, text=label, width=15, command=func,
                            bg=color, fg=COLORS['text'],
                            activebackground=COLORS['button_active'],
                            activeforeground=COLORS['text'],
                            relief=tk.RAISED, bd=2, font=('Arial', 10, 'bold'))
            btn.grid(row=i // 3, column=i % 3, padx=5, pady=5, sticky='ew')

        # Configure grid columns to expand
        for i in range(3):
            button_frame.columnconfigure(i, weight=1)

        # Create notebook for tabs
        self.notebook = ttk.Notebook(master)
        self.notebook.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)

        # Data tab (console)
        self.data_tab = ttk.Frame(self.notebook, style='TFrame')
        self.notebook.add(self.data_tab, text="Data")

        # Scrolled text area to display serial output
        self.text_area = scrolledtext.ScrolledText(self.data_tab, width=80, height=20,
                                                   bg=COLORS['graph_bg'], fg=COLORS['text'],
                                                   insertbackground=COLORS['text'])
        self.text_area.pack(padx=5, pady=5, fill=tk.BOTH, expand=True)
        self.text_area.configure(font=('Consolas', 10))

        # Map tab
        self.map_tab = ttk.Frame(self.notebook, style='TFrame')
        self.notebook.add(self.map_tab, text="Map")

        # Add map view to map tab
        self.setup_map()

        # Graphs tab
        self.graphs_tab = ttk.Frame(self.notebook, style='TFrame')
        self.notebook.add(self.graphs_tab, text="Graphs")

        # Set up the graphs
        self.setup_graphs()

        # Debug tab
        self.debug_tab = ttk.Frame(self.notebook, style='TFrame')
        self.notebook.add(self.debug_tab, text="Debug")

        # Debug text area
        self.debug_text_area = scrolledtext.ScrolledText(self.debug_tab, width=80, height=20,
                                                         bg=COLORS['graph_bg'], fg=COLORS['warning'],
                                                         insertbackground=COLORS['text'])
        self.debug_text_area.pack(padx=5, pady=5, fill=tk.BOTH, expand=True)
        self.debug_text_area.configure(font=('Consolas', 10))

        # Debug controls
        debug_control_frame = tk.Frame(self.debug_tab, bg=COLORS['background'])
        debug_control_frame.pack(pady=5, fill=tk.X)

        self.toggle_debug_button = tk.Button(debug_control_frame, text="Toggle Debug Mode",
                                             command=self.toggle_debug_mode,
                                             bg=COLORS['warning'], fg=COLORS['text'])
        self.toggle_debug_button.pack(side=tk.LEFT, padx=10)

        self.clear_debug_button = tk.Button(debug_control_frame, text="Clear Debug Log",
                                            command=self.clear_debug_log,
                                            bg=COLORS['error'], fg=COLORS['text'])
        self.clear_debug_button.pack(side=tk.LEFT, padx=10)

        # Buttons to switch tabs
        tab_button_frame = tk.Frame(master, bg=COLORS['background'])
        tab_button_frame.pack(padx=10, pady=5, fill=tk.X)

        self.data_button = tk.Button(tab_button_frame, text="Data View",
                                     command=lambda: self.notebook.select(0),
                                     bg=COLORS['button_bg'], fg=COLORS['text'],
                                     activebackground=COLORS['button_active'])
        self.data_button.pack(side=tk.LEFT, padx=5, pady=5)

        self.map_button = tk.Button(tab_button_frame, text="Map View",
                                    command=lambda: self.notebook.select(1),
                                    bg=COLORS['button_bg'], fg=COLORS['text'],
                                    activebackground=COLORS['button_active'])
        self.map_button.pack(side=tk.LEFT, padx=5, pady=5)

        self.graphs_button = tk.Button(tab_button_frame, text="Graphs View",
                                       command=lambda: self.notebook.select(2),
                                       bg=COLORS['button_bg'], fg=COLORS['text'],
                                       activebackground=COLORS['button_active'])
        self.graphs_button.pack(side=tk.LEFT, padx=5, pady=5)

        self.debug_button = tk.Button(tab_button_frame, text="Debug View",
                                      command=lambda: self.notebook.select(3),
                                      bg=COLORS['warning'], fg=COLORS['text'],
                                      activebackground=COLORS['button_active'])
        self.debug_button.pack(side=tk.LEFT, padx=5, pady=5)

        # Initialize serial connection
        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            self.text_area.insert(tk.END, f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud.\n")
            self.update_status(connected=True)
        except Exception as e:
            self.ser = None
            self.text_area.insert(tk.END, f"Error opening serial port: {e}\n")
            self.update_status(connected=False, message=str(e))

        # Start thread to continuously read from the serial port
        self.running = True
        if self.ser is not None:
            self.read_thread = threading.Thread(target=self.read_serial)
            self.read_thread.daemon = True
            self.read_thread.start()

        # Log startup
        self.log_debug("Application started. Debug mode enabled.")

        # Show splash screen
        self.splash = SplashScreen(master)

    def log_debug(self, message):
        """Log a debug message to the debug console"""
        if self.debug_mode:
            timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
            debug_message = f"[{timestamp}] {message}"
            self.debug_text_area.insert(tk.END, debug_message + "\n")
            self.debug_text_area.see(tk.END)

    def clear_debug_log(self):
        """Clear the debug log"""
        self.debug_text_area.delete(1.0, tk.END)
        self.log_debug("Debug log cleared")

    def toggle_debug_mode(self):
        """Toggle debug mode on/off"""
        self.debug_mode = not self.debug_mode
        if self.debug_mode:
            self.debug_indicator.pack(side=tk.LEFT, padx=15)
            self.log_debug("Debug mode enabled")
        else:
            self.debug_indicator.pack_forget()
            self.log_debug("Debug mode disabled")

    def update_status(self, connected=False, message=None):
        """Update the connection status indicator"""
        if connected:
            self.status_indicator.config(fg=COLORS['success'])
            self.status_text.config(text="Connected")
        else:
            self.status_indicator.config(fg=COLORS['error'])
            self.status_text.config(text=f"Disconnected{f': {message}' if message else ''}")

    def update_mission_timer(self):
        """Update the mission timer display"""
        if self.flight_mode_active and self.launch_time:
            mission_seconds = time.time() - self.launch_time
            timer_text = self.format_mission_time(mission_seconds)
            self.mission_timer.config(text=timer_text)

        # Schedule the next update
        self.master.after(1000, self.update_mission_timer)

    def setup_graphs(self):
        """Setup the graphs in the graphs tab"""
        # Create a frame with a scrollbar for all the graphs
        graph_container = ttk.Frame(self.graphs_tab, style='TFrame')
        graph_container.pack(fill=tk.BOTH, expand=True)

        # Create a canvas with scrollbar
        canvas = tk.Canvas(graph_container, bg=COLORS['background'],
                           highlightthickness=0)
        scrollbar = ttk.Scrollbar(graph_container, orient="vertical", command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas, style='TFrame')

        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )

        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        # Clear any existing graph axes
        self.graph_axes = []

        # Create graphs with data mappings
        self.create_graph(scrollable_frame, "Temperature (°C)",
                          self.data_temperature, self.time_temperature, GRAPH_COLORS['temperature'])
        self.create_graph(scrollable_frame, "Humidity (%)",
                          self.data_humidity, self.time_humidity, GRAPH_COLORS['humidity'])
        self.create_graph(scrollable_frame, "Pressure (hPa)",
                          self.data_pressure, self.time_pressure, GRAPH_COLORS['pressure'])
        self.create_graph(scrollable_frame, "CO2 (ppm)",
                          self.data_co2, self.time_co2, GRAPH_COLORS['co2'])
        self.create_graph(scrollable_frame, "Altitude (m)",
                          self.data_altitude, self.time_altitude, GRAPH_COLORS['altitude'])
        self.create_graph(scrollable_frame, "Acceleration (m/s²)",
                          self.data_acceleration, self.time_acceleration, GRAPH_COLORS['acceleration'])
        self.create_graph(scrollable_frame, "MQ135 Gas",
                          self.data_mq135, self.time_mq135, GRAPH_COLORS['mq135'])
        self.create_graph(scrollable_frame, "MQ4 Gas",
                          self.data_mq4, self.time_mq4, GRAPH_COLORS['mq4'])
        self.create_graph(scrollable_frame, "Light (LDR)",
                          self.data_ldr, self.time_ldr, GRAPH_COLORS['ldr'])

        # Button frame for graph controls
        button_frame = ttk.Frame(self.graphs_tab, style='TFrame')
        button_frame.pack(pady=5, fill=tk.X)

        # Button to refresh graphs
        self.refresh_button = tk.Button(button_frame, text="Refresh Graphs",
                                        command=self.update_all_graphs,
                                        bg=COLORS['button_bg'], fg=COLORS['text'])
        self.refresh_button.pack(side=tk.LEFT, padx=10)

        # Button to clear all graph data
        self.clear_button = tk.Button(button_frame, text="Clear Graphs",
                                      command=self.clear_graph_data,
                                      bg=COLORS['warning'], fg=COLORS['text'])
        self.clear_button.pack(side=tk.LEFT, padx=10)

    def create_graph(self, parent, title, data_source, time_source, color):
        """Create a single graph in the parent frame"""
        frame = ttk.LabelFrame(parent, text=title, style='Graph.TLabelframe')
        frame.pack(padx=10, pady=10, fill=tk.X)

        # Configure matplotlib to match our theme
        plt.rcParams.update({
            'axes.facecolor': COLORS['graph_bg'],
            'figure.facecolor': COLORS['graph_bg'],
            'text.color': COLORS['text'],
            'axes.labelcolor': COLORS['text'],
            'axes.edgecolor': COLORS['graph_grid'],
            'xtick.color': COLORS['text'],
            'ytick.color': COLORS['text'],
            'grid.color': COLORS['graph_grid'],
            'grid.linestyle': '--',
            'grid.alpha': 0.7
        })

        fig, ax = plt.subplots(figsize=(8, 3))
        line, = ax.plot([], [], color=color, marker='o', linestyle='-',
                        linewidth=2, markersize=4, markerfacecolor='white')
        ax.set_xlabel('Time (seconds)')
        ax.set_ylabel(title)
        ax.grid(True)

        # Add a bit of padding to the y-axis limits
        ax.margins(y=0.1)

        # Create the matplotlib canvas
        canvas = FigureCanvasTkAgg(fig, master=frame)
        canvas.draw()
        canvas.get_tk_widget().pack(fill=tk.X)

        # Add value display
        value_frame = tk.Frame(frame, bg=COLORS['graph_bg'])
        value_frame.pack(fill=tk.X, pady=(0, 5))

        tk.Label(value_frame, text="Current:", bg=COLORS['graph_bg'],
                 fg=COLORS['text']).pack(side=tk.LEFT, padx=5)

        current_value = tk.Label(value_frame, text="No data", bg=COLORS['graph_bg'],
                                 fg=color, font=('Arial', 10, 'bold'))
        current_value.pack(side=tk.LEFT, padx=5)

        # Store the figure, axis, line, and data source for updates
        fig.tight_layout()
        ax.set_title(title, color=COLORS['text'])

        # Store references for updating
        ax.data_source = data_source
        ax.time_source = time_source
        ax.line = line
        ax.current_value_label = current_value
        ax.graph_color = color

        # Add this axis to our list of graphs
        self.graph_axes.append(ax)

        return ax

    def update_graph(self, ax):
        """Update a single graph with new data"""
        data = list(ax.data_source)
        times = list(ax.time_source)

        if len(data) > 0 and len(times) > 0 and len(data) == len(times):
            ax.line.set_data(times, data)
            ax.relim()
            ax.autoscale_view()

            # Update the current value display
            current_value = data[-1] if data else "No data"
            ax.current_value_label.config(text=f"{current_value}")

            ax.figure.canvas.draw_idle()
        else:
            # For debugging
            if self.debug_mode:
                self.log_debug(f"Data issue in graph {ax.get_title()}: len(data)={len(data)}, len(times)={len(times)}")

    def update_all_graphs(self):
        """Update all graphs with current data"""
        for ax in self.graph_axes:
            self.update_graph(ax)

    def clear_graph_data(self):
        """Clear all graph data"""
        # Confirm with the user
        if messagebox.askyesno("Clear Graphs", "Are you sure you want to clear all graph data?"):
            # Clear all data deques
            self.data_temperature.clear()
            self.data_humidity.clear()
            self.data_pressure.clear()
            self.data_co2.clear()
            self.data_altitude.clear()
            self.data_acceleration.clear()
            self.data_mq135.clear()
            self.data_mq4.clear()
            self.data_ldr.clear()

            # Clear all time deques
            self.time_temperature.clear()
            self.time_humidity.clear()
            self.time_pressure.clear()
            self.time_co2.clear()
            self.time_altitude.clear()
            self.time_acceleration.clear()
            self.time_mq135.clear()
            self.time_mq4.clear()
            self.time_ldr.clear()

            # Reset the first data time
            self.first_data_time = None

            # Reset last values
            self.last_values = {key: None for key in self.last_values}

            # Update all graphs
            self.update_all_graphs()

            # Show confirmation
            self.text_area.insert(tk.END, "All graph data has been cleared.\n")
            self.text_area.see(tk.END)

    def setup_map(self):
        """Setup the map view"""
        # Create a frame for the map
        map_frame = ttk.Frame(self.map_tab, style='TFrame')
        map_frame.pack(fill=tk.BOTH, expand=True)

        # Create map widget
        self.map_widget = tkintermapview.TkinterMapView(map_frame, width=800, height=500, corner_radius=0)
        self.map_widget.pack(fill=tk.BOTH, expand=True)

        # Set default position and zoom
        self.map_widget.set_position(60.17, 24.95)  # Default: Helsinki
        self.map_widget.set_zoom(13)

        # Add a label to show current coordinates
        self.coords_frame = tk.Frame(self.map_tab, bg=COLORS['background'])
        self.coords_frame.pack(fill=tk.X, pady=5)

        tk.Label(self.coords_frame, text="Position:", bg=COLORS['background'],
                 fg=COLORS['text']).pack(side=tk.LEFT, padx=5)

        self.coords_label = tk.Label(self.coords_frame, text="No GPS data",
                                     bg=COLORS['background'], fg=COLORS['accent'],
                                     font=('Arial', 10))
        self.coords_label.pack(side=tk.LEFT, padx=5)

        # Create a path object for the flight path
        self.path = None

        # Last marker for current position
        self.current_marker = None

    def update_map(self, latitude, longitude, altitude):
        """Update the map with new GPS coordinates"""
        # Check if coordinates are valid (not 0,0)
        if abs(latitude) < 0.1 and abs(longitude) < 0.1:
            return  # Skip invalid coordinates

        # Store coordinates for path
        self.gps_coordinates.append((latitude, longitude))

        # Update the coordinates label
        self.coords_label.config(text=f"{latitude:.6f}, {longitude:.6f}, Alt: {altitude:.1f}m")

        # Update the map position to show current location
        self.map_widget.set_position(latitude, longitude)

        # Add or update marker for current position
        if self.current_marker:
            self.current_marker.delete()
        self.current_marker = self.map_widget.set_marker(latitude, longitude, text=f"Alt: {altitude:.1f}m")

        # Update the path if we have at least 2 coordinates
        if len(self.gps_coordinates) >= 2:
            if self.path:
                self.path.delete()
            self.path = self.map_widget.set_path(self.gps_coordinates)

    def send_serial_command(self, command):
        """Send the specified command over the serial port."""
        if self.ser is not None:
            try:
                self.ser.write((command + "\n").encode())
                message = f"Sent: {command}"
                self.text_area.insert(tk.END, message + "\n")
                self.text_area.tag_config("command", foreground=COLORS['success'])
                self.text_area.tag_add("command", f"end-{len(message) + 1}c", "end-1c")
                self.text_area.see(tk.END)

                # Update mission time tracking
                if command == "FLIGHT" and not self.flight_mode_active:
                    self.launch_time = time.time()
                    self.flight_mode_active = True
                    self.text_area.insert(tk.END, "Mission timer started.\n")
                elif command == "RECOVERY" or command == "PRELAUNCH":
                    self.flight_mode_active = False
                    if command == "RECOVERY":
                        self.text_area.insert(tk.END, "Mission timer stopped.\n")

            except Exception as e:
                error_msg = f"Error sending command '{command}': {e}"
                self.text_area.insert(tk.END, error_msg + "\n")
                self.text_area.tag_config("error", foreground=COLORS['error'])
                self.text_area.tag_add("error", f"end-{len(error_msg) + 1}c", "end-1c")

    def send_custom_command(self):
        """Send a custom command from the entry box."""
        command = self.command_entry.get()
        if command:
            self.send_serial_command(command)
            self.command_entry.delete(0, tk.END)

    # Command button methods
    def send_prelaunch(self):
        self.send_serial_command("PRELAUNCH")

    def send_flight(self):
        self.send_serial_command("FLIGHT")

    def send_recovery(self):
        self.send_serial_command("RECOVERY")

    def send_init_scd40(self):
        self.send_serial_command("INIT_SCD40")

    def send_init_gps(self):
        self.send_serial_command("INIT_GPS")

    def send_init_mq(self):
        self.send_serial_command("INIT_MQ")

    def send_init_board(self):
        self.send_serial_command("INIT_BOARD")

    def format_mission_time(self, seconds):
        """Format mission time in MM:SS format"""
        minutes = int(seconds / 60)
        seconds = int(seconds % 60)
        return f"T+{minutes:02d}:{seconds:02d}"

    def parse_csv_data(self, line):
        """Parse a CSV line of sensor data."""
        try:
            if line.startswith("CSV:"):
                data = line[4:].strip().split(',')
                if len(data) >= 17:  # We now expect 17+ values including mission time
                    parsed_data = {
                        'timestamp': int(data[0]),
                        'mission_time': int(data[1]),
                        'co2': int(data[2]),
                        'temperature': float(data[3]),
                        'humidity': float(data[4]),
                        'mq135': int(data[5]),
                        'mq4': int(data[6]),
                        'latitude': float(data[7]),
                        'longitude': float(data[8]),
                        'altitude': float(data[9]),
                        'board_temp': float(data[10]),
                        'pressure': float(data[11]),
                        'ldr': float(data[12]),
                        'acceleration': float(data[13]),
                        'acc_x': float(data[14]),
                        'acc_y': float(data[15]),
                        'acc_z': float(data[16])
                    }

                    # Log raw values for pressure and MQ4 in debug mode
                    if self.debug_mode:
                        self.log_debug(f"Parsed data: Pressure={parsed_data['pressure']}, MQ4={parsed_data['mq4']}")

                    return parsed_data
            return None
        except Exception as e:
            self.text_area.insert(tk.END, f"Error parsing CSV data: {e}\n")
            if self.debug_mode:
                self.log_debug(f"CSV Parse Error: {e} in line: {line}")
            return None

    def get_elapsed_seconds(self):
        """Get seconds elapsed since first data point"""
        now = time.time()
        if self.first_data_time is None:
            self.first_data_time = now
            return 0
        return round(now - self.first_data_time, 1)

    def is_valid_scd40_data(self, parsed_data):
        """Check if SCD40 data appears valid (not zero/default values)"""
        # Check if CO2 is within a reasonable range and not just a default value
        # Normal CO2 outdoors is around 400ppm, indoors can be higher
        # We'll consider readings below 350 or above 10000 as potentially invalid
        co2_valid = 350 <= parsed_data['co2'] <= 10000

        # Check if temperature is within a reasonable range
        # -40 to 85 is the operational range of most sensors
        temp_valid = -40 <= parsed_data['temperature'] <= 85

        # Check if humidity is within a valid range (0-100%)
        humidity_valid = 0 <= parsed_data['humidity'] <= 100

        # All three values should be valid and at least one should have changed
        is_valid = (co2_valid and temp_valid and humidity_valid and
                    (self.last_values['co2'] != parsed_data['co2'] or
                     self.last_values['temperature'] != parsed_data['temperature'] or
                     self.last_values['humidity'] != parsed_data['humidity']))

        if not is_valid and self.debug_mode:
            self.log_debug(f"Invalid SCD40 data: CO2={parsed_data['co2']} ({co2_valid}), "
                           f"Temp={parsed_data['temperature']} ({temp_valid}), "
                           f"Humidity={parsed_data['humidity']} ({humidity_valid})")

        return is_valid

    def update_graph_data(self, parsed_data):
        """Update the data for graphs, but only if values changed and are valid"""
        # Get current elapsed time
        elapsed_seconds = self.get_elapsed_seconds()

        # Check if SCD40 data is valid before adding
        scd40_valid = self.is_valid_scd40_data(parsed_data)

        # Temperature, CO2, and Humidity are from SCD40
        if scd40_valid:
            # Temperature
            if self.last_values['temperature'] != parsed_data['temperature']:
                self.data_temperature.append(parsed_data['temperature'])
                self.time_temperature.append(elapsed_seconds)
                self.last_values['temperature'] = parsed_data['temperature']
                self.flash_success("New temperature reading")

            # Humidity
            if self.last_values['humidity'] != parsed_data['humidity']:
                self.data_humidity.append(parsed_data['humidity'])
                self.time_humidity.append(elapsed_seconds)
                self.last_values['humidity'] = parsed_data['humidity']
                self.flash_success("New humidity reading")

            # CO2
            if self.last_values['co2'] != parsed_data['co2']:
                self.data_co2.append(parsed_data['co2'])
                self.time_co2.append(elapsed_seconds)
                self.last_values['co2'] = parsed_data['co2']
                self.flash_success("New CO2 reading")

        # For non-SCD40 sensors, continue using the previous approach but with expanded/disabled range checks
        # Pressure (from different sensor) - EXPANDED RANGE
        if self.last_values['pressure'] != parsed_data['pressure']:
            # Accept any non-zero pressure value for debugging
            if parsed_data['pressure'] != 0:
                self.data_pressure.append(parsed_data['pressure'])
                self.time_pressure.append(elapsed_seconds)
                self.last_values['pressure'] = parsed_data['pressure']
                if self.debug_mode:
                    self.log_debug(f"Added pressure data: {parsed_data['pressure']}")
                self.flash_success("New pressure reading")
            else:
                if self.debug_mode:
                    self.log_debug(f"Rejected zero pressure value")

        # Altitude
        if self.last_values['altitude'] != parsed_data['altitude']:
            # Reasonable range check, adjust based on your mission
            if -100 <= parsed_data['altitude'] <= 10000:
                self.data_altitude.append(parsed_data['altitude'])
                self.time_altitude.append(elapsed_seconds)
                self.last_values['altitude'] = parsed_data['altitude']
                self.flash_success("New altitude reading")

        # Acceleration
        if self.last_values['acceleration'] != parsed_data['acceleration']:
            if -20 <= parsed_data['acceleration'] <= 20:
                self.data_acceleration.append(parsed_data['acceleration'])
                self.time_acceleration.append(elapsed_seconds)
                self.last_values['acceleration'] = parsed_data['acceleration']
                self.flash_success("New acceleration reading")

        # MQ135
        if self.last_values['mq135'] != parsed_data['mq135']:
            # Accept any value for debugging
            self.data_mq135.append(parsed_data['mq135'])
            self.time_mq135.append(elapsed_seconds)
            self.last_values['mq135'] = parsed_data['mq135']
            if self.debug_mode:
                self.log_debug(f"Added MQ135 data: {parsed_data['mq135']}")
            self.flash_success("New MQ135 reading")

        # MQ4 - REMOVED RANGE CHECK
        if self.last_values['mq4'] != parsed_data['mq4']:
            # Accept any value for debugging
            self.data_mq4.append(parsed_data['mq4'])
            self.time_mq4.append(elapsed_seconds)
            self.last_values['mq4'] = parsed_data['mq4']
            if self.debug_mode:
                self.log_debug(f"Added MQ4 data: {parsed_data['mq4']}")
            self.flash_success("New MQ4 reading")

        # LDR
        if self.last_values['ldr'] != parsed_data['ldr']:
            # Accept any value for debugging
            self.data_ldr.append(parsed_data['ldr'])
            self.time_ldr.append(elapsed_seconds)
            self.last_values['ldr'] = parsed_data['ldr']
            if self.debug_mode:
                self.log_debug(f"Added LDR data: {parsed_data['ldr']}")
            self.flash_success("New LDR reading")

        # If we're on the graphs tab, update the graphs
        if self.notebook.index(self.notebook.select()) == 2:
            self.master.after(100, self.update_all_graphs)

    def flash_success(self, message=None):
        """Flash the status indicator for successful data reception"""
        # No visual feedback for now to avoid distractions
        if self.debug_mode and message:
            self.log_debug(f"Data received: {message}")

    def read_serial(self):
        """Continuously read data from the serial port, log it, and display it."""
        with open(LOG_FILE, "a") as log_file:
            while self.running:
                if self.ser is not None and self.ser.in_waiting:
                    try:
                        line = self.ser.readline().decode(errors="replace").strip()
                        if line:
                            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

                            # Check for launch time set message
                            if "LAUNCH_TIME_SET" in line:
                                self.launch_time = time.time()
                                self.flight_mode_active = True
                                message = f"{timestamp} - Mission timer started"
                                self.text_area.insert(tk.END, message + "\n")
                                self.text_area.tag_config("mission", foreground=COLORS['success'])
                                self.text_area.tag_add("mission", f"end-{len(message) + 1}c", "end-1c")
                                log_file.write(f"{timestamp}:MISSION_START\n")
                                continue

                            # Add mission time if we're in flight mode
                            mission_time_str = ""
                            if self.flight_mode_active and self.launch_time:
                                mission_seconds = time.time() - self.launch_time
                                mission_time_str = f" [{self.format_mission_time(mission_seconds)}]"

                            # Display in GUI
                            display_line = f"{timestamp}{mission_time_str} - {line}"
                            self.text_area.insert(tk.END, display_line + "\n")
                            self.text_area.see(tk.END)

                            # Standard log line
                            log_file.write(f"{timestamp}:{line}\n")
                            log_file.flush()

                            # Parse CSV data if present
                            if line.startswith("CSV:"):
                                parsed_data = self.parse_csv_data(line)
                                if parsed_data:
                                    # Create a formatted data string for the main log file
                                    formatted_log = (
                                        f"{timestamp}:{parsed_data['temperature']}:"
                                        f"{parsed_data['pressure']}:{parsed_data['co2']}:"
                                        f"{parsed_data['humidity']}:{parsed_data['mq135']}:"
                                        f"{parsed_data['mq4']}:{parsed_data['latitude']}:"
                                        f"{parsed_data['longitude']}:{parsed_data['altitude']}:"
                                        f"{parsed_data['ldr']}:{parsed_data['acceleration']}:"
                                        f"{parsed_data['acc_x']}:{parsed_data['acc_y']}:"
                                        f"{parsed_data['acc_z']}"
                                    )
                                    # Write the formatted data to the log file
                                    log_file.write(f"DATA:{formatted_log}\n")
                                    log_file.flush()

                                    # Update map with GPS coordinates if they're valid
                                    if parsed_data['latitude'] != 0 or parsed_data['longitude'] != 0:
                                        self.master.after(10, lambda lat=parsed_data['latitude'],
                                                                     lon=parsed_data['longitude'],
                                                                     alt=parsed_data['altitude']:
                                        self.update_map(lat, lon, alt))

                                    # Update graph data
                                    self.update_graph_data(parsed_data)

                    except Exception as e:
                        error_msg = f"Error reading serial data: {e}"
                        self.text_area.insert(tk.END, error_msg + "\n")
                        self.text_area.tag_config("error", foreground=COLORS['error'])
                        self.text_area.tag_add("error", f"end-{len(error_msg) + 1}c", "end-1c")
                        log_file.write(f"{timestamp}:ERROR:{error_msg}\n")
                        log_file.flush()
                        if self.debug_mode:
                            self.log_debug(f"Serial read error: {e}")
                time.sleep(0.1)

    def on_close(self):
        """Cleanup the serial connection and exit."""
        self.running = False
        if self.ser is not None:
            self.ser.close()
        self.master.destroy()


def main():
    root = tk.Tk()

    # Check for required packages
    try:
        import tkintermapview
        import matplotlib
        import PIL
    except ImportError as e:
        error_window = tk.Toplevel(root)
        error_window.title("Missing Package")
        tk.Label(error_window, text=f"The following package is not installed: {e.name}").pack(pady=10)
        tk.Label(error_window, text="Please install it with:").pack()
        tk.Label(error_window, text=f"pip install {e.name}").pack(pady=10)
        tk.Button(error_window, text="OK", command=error_window.destroy).pack(pady=10)

    app = CanSatGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()


if __name__ == "__main__":
    main()
