import sys
import termios
import tty
import socket

# UDP server IP and port
UDP_IP = "192.168.2.2"  # Replace with the IP address of the Raspberry Pi
UDP_IP = "192.168.0.80"  # Replace with the IP address of the Raspberry Pi
UDP_PORT = 5005

# UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Function to send a UDP command
def send_command(command, file_name):
    print(f"Sending command: {command} {file_name}")
    sock.sendto(f"{command} {file_name}".encode("utf-8"), (UDP_IP, UDP_PORT))

# Function to get a single character without requiring Enter key
def get_char():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

# Print instructions
print("UDP Audio Client")
print("----------------")
print("Instructions:")
print("- Press a numeric key (1-9) to play the corresponding WAV file.")
print("- Press '0' to stop all playing WAV files.")
print("- Press 'q' to quit.")

# Client loop
while True:
    try:
        # Wait for a keypress
        key = get_char()

        if key == "q":
            break

        if key == "0":
            send_command("stopall", "")
        elif key.isdigit() and 1 <= int(key) <= 9:
            file_name = f"{key}.wav"
            send_command("start", file_name)
        else:
            print("Invalid input. Please press a numeric key between 1 and 9, '0' to stop all, or 'q' to quit.")

    except KeyboardInterrupt:
        break

sock.close()
