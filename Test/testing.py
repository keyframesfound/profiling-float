import serial
import time

# Configure serial connection parameters
SERIAL_PORT = '/dev/tty.usbserial'  # Replace with your ESP8266's serial port
BAUD_RATE = 115200

def send_receive_message():import serial
import serial.tools.list_ports
import time

def list_serial_ports():
    """List all available serial ports"""
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("No serial ports found!")
        return None
    
    print("\nAvailable serial ports:")
    for index, port in enumerate(ports):
        print(f"{index}: {port.device} - {port.description}")
    
    while True:
        try:
            choice = int(input("\nSelect port number (or -1 to exit): "))
            if choice == -1:
                return None
            if 0 <= choice < len(ports):
                return ports[choice].device
        except ValueError:
            print("Please enter a valid number")

# Configure serial connection parameters
BAUD_RATE = 115200

def send_receive_message(port):
    try:
        # Open serial connection
        ser = serial.Serial(
    try:
        # Open serial connection
        ser = serial.Serial(
            port=SERIAL_PORT,
            baudrate=BAUD_RATE,
            timeout=1,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )
        
        # Send message to ESP8266
        message = "Hello world"
        ser.write(message.encode())
        print(f"Sent: {message}")
        
        # Receive response from ESP8266
        response = ser.readline().decode().strip()
        print(f"Received: {response}")
        
        # Close the connection
        ser.close()
        
    except Exception as e:
        print(f"Error: {e}")

# Main loop
while True:
    send_receive_message()
    time.sleep(5)  # Wait 5 seconds before sending next message
