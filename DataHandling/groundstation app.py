import serial
import threading
import time
import os
from prompt_toolkit import prompt
from pathlib import Path
from prompt_toolkit.patch_stdout import patch_stdout

# Update with your device's serial port and baud rate
SERIAL_PORT = "COM6"  # or "/dev/ttyUSB0" on Linux
BAUD_RATE = 115200

cur_dir=Path.cwd()
CSV_FILE=r"C:\Users\oskar.huotari\Documents\Arduino\python app\data.csv"

def ensure_csv_file():
    """
    Ensure the CSV file exists with the correct header.
    """
    if not os.path.exists(CSV_FILE):
        try:
            with open(CSV_FILE, "w") as file:
                header = ("scd40_error,timestamp,mission_time,co2,temperature,humidity,"
                          "mq135,mq4,latitude,longitude,altitude,board_temperature,board_pressure,"
                          "board_ldr,board_acceleration,board_acc_x,board_acc_y,board_acc_z\n")
                file.write(header)
            print(f"CSV file created: {os.path.abspath(CSV_FILE)}")
        except Exception as e:
            print("Error creating CSV file:", e)
    else:
        print(f"CSV file already exists: {os.path.abspath(CSV_FILE)}")

def write_csv_line(csv_line):
    """
    Append the CSV line (without the "CSV:" prefix) to the CSV file.
    """
    try:
        with open(CSV_FILE, "a") as file:
            file.write(csv_line + "\n")
    except Exception as e:
        print("Error writing to CSV file:", e)

def read_serial(ser):
    """
    Continuously read from the serial port.
    If a line starts with "CSV:", write the payload to the CSV file.
    Otherwise, print the line.
    """
    while True:
        try:
            line = ser.readline().decode('utf-8', errors='replace').strip()
            if line:
                if line.startswith("CSV:"):
                    csv_line = line[len("CSV:"):].strip()
                    write_csv_line(csv_line)
                else:
                    print(line)
        except Exception as e:
            print("Error reading from serial port:", e)
            break

def send_commands(ser):
    """
    Read user input continuously and send it to the serial port.
    Uses prompt_toolkit to avoid interference with printed output.
    """
    while True:
        try:
            with patch_stdout():
                user_input = prompt("Enter command: ")
            if user_input:
                ser.write((user_input + "\n").encode('utf-8'))
        except Exception as e:
            print("Error sending command:", e)
            break

def main():
    ensure_csv_file()  # Ensure CSV file is created and has a header.
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Allow time for the connection to initialize
        print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud.")

        # Start a thread to continuously read from the serial port
        read_thread = threading.Thread(target=read_serial, args=(ser,), daemon=True)
        read_thread.start()

        # Main thread handles sending commands using prompt_toolkit
        send_commands(ser)
    except Exception as e:
        print("Failed to connect to serial port:", e)

if __name__ == "__main__":
    main()
