import socket
import time

# Configure connection parameters
ESP_IP = '192.168.1.100'  # Replace with your ESP8266's IP address
ESP_PORT = 80
BUFFER_SIZE = 1024

def send_receive_message():
    try:
        # Create a TCP socket
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((ESP_IP, ESP_PORT))
        
        # Send message to ESP8266
        message = "Hello world"
        client_socket.send(message.encode())
        print(f"Sent: {message}")
        
        # Receive response from ESP8266
        response = client_socket.recv(BUFFER_SIZE).decode()
        print(f"Received: {response}")
        
        # Close the connection
        client_socket.close()
        
    except Exception as e:
        print(f"Error: {e}")

# Main loop
while True:
    send_receive_message()
    time.sleep(5)  # Wait 5 seconds before sending next message