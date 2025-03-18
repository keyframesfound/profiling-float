import serial
import serial.tools.list_ports
import time
import sys

BAUD_RATE = 115200

def list_serial_ports():
    """List all available serial ports"""
    print("Scanning for serial ports...")
    ports = serial.tools.list_ports.comports()
    
    if not ports:
        print("No serial ports found! Please check your connections.")
        return None
    
    print("\nFound the following serial ports:")
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

def send_receive_message(port):
    try:
        print(f"Opening serial port: {port}")
        ser = serial.Serial(
            port=port,
            baudrate=BAUD_RATE,
            timeout=1,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )
        
        message = "D100"
        print(f"Attempting to send: {message.strip()}")
        sent_bytes = ser.write(message.encode())
        if sent_bytes == len(message):
            print(f"Sent: {message.strip()}")
        else:
            print("NOT SENT")
        
        print("Waiting for response...")
        response = ser.readline().decode().strip()
        if response == "D100":
            print("D100 Received")
        elif response:
            print(f"Received: {response}")
        else:
            print("No response received")
        
        ser.close()
        print(f"Closed serial port: {port}")
        
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    selected_port = list_serial_ports()
    if selected_port:
        print(f"Selected port: {selected_port}")
        while True:
            send_receive_message(selected_port)
            time.sleep(5)
    else:
        print("No port selected. Exiting...")