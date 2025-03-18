import serial
import time
from serial.tools.list_ports import comports

# Since only one serial device is connected, use its known device name.
SERIAL_PORT = ''  # Replace with the exact device name if needed.
BAUD_RATE = 9600

# New auto-detect serial port logic.
ports = list(comports())
if len(ports) == 0:
    print("No serial ports found. Exiting.")
    exit(1)
elif len(ports) == 1:
    SERIAL_PORT = ports[0].device
else:
    print("Multiple serial ports found:")
    for idx, port in enumerate(ports):
        print(f"{idx}: {port.device} - {port.description}")
    idx = int(input("Select port index: ").strip())
    SERIAL_PORT = ports[idx].device

def main():
    ser = None
    while True:
        max_attempts = 3
        attempts = 0
        while attempts < max_attempts:
            try:
                ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
                print(f"Connected to ESP device on {SERIAL_PORT}.")
                break
            except Exception as e:
                if "Resource busy" in str(e):
                    print(f"Port busy: retrying ({attempts+1}/{max_attempts})...")
                    time.sleep(2)
                    attempts += 1
                else:
                    print("Error:", e)
                    return
        if ser:
            # Flush any stale data from the buffer.
            ser.reset_input_buffer()
            break
        else:
            choice = input("Failed to open port after retries. Press 'r' to retry again, any other key to exit: ").strip().lower()
            if choice != "r":
                return
            print("Retrying to open the port...")

    # New code: interactive prompt.
    user_input = input("Enter command and value (e.g. U 1000, D 100, d 500): ").strip().split()
    if len(user_input) != 2:
        print("Invalid input format. Expected: <command> <value>")
        return

    command_char, value = user_input
    # Append newline for proper termination.
    command_signal = f"{command_char}{value}\n"
    ser.write(command_signal.encode())
    print("Sent:", command_signal)
    
    # Wait a short moment for ESP board to process & respond.
    time.sleep(0.2)
    
    # New code: read all available response lines.
    while ser.in_waiting:
        response = ser.readline().decode().strip()
        if response:
            print("Received:", response)

    if ser.is_open:
        ser.close()

if __name__ == "__main__":
    main()