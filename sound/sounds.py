import os
import socket
import simpleaudio as sa
import random
import threading
import time

# Dictionary to store WAV file objects
wav_files = {}
play_objects = {}

# Configurable settings
wav_directory = "/home/mutopia/wav"  # Default directory path containing WAV files
random_idle_time = 30  # Configurable time in seconds before playing random sounds
min_random_delay = 5  # Minimum delay before playing random sounds in seconds
max_random_delay = 20  # Maximum delay before playing random sounds in seconds

# Lock for handling random sound playback
random_play_lock = threading.Lock()

# Time of the last received UDP command
last_command_time = time.time()

# Function to play a WAV file
def play_wav(file_name):
    if file_name in wav_files:
        if file_name in play_objects and play_objects[file_name].is_playing():
            print(f"WAV file '{file_name}' is already playing.")
            return

        wav_obj = wav_files[file_name]
        play_obj = wav_obj.play()
        play_objects[file_name] = play_obj
        print(f"Playing WAV file: {file_name}")
    else:
        print(f"Error: WAV file '{file_name}' not found.")

# Function to stop playing all WAV files
def stop_all():
    sa.stop_all()
    play_objects.clear()

# Function to play random sounds at random intervals
def play_random_sounds():
    while True:
        current_time = time.time()
        if current_time - last_command_time > random_idle_time:
            random_play_lock.acquire()
            random_file = random.choice(list(wav_files.keys()))
            print(f"Playing random sound: {random_file}")
            play_wav(random_file)
            random_play_lock.release()

        random_delay = random.randint(min_random_delay, max_random_delay)
        time.sleep(random_delay)

# UDP packet listener
def listen_udp():
    # Define UDP server IP and port
    UDP_IP = "0.0.0.0"  # Listen on all network interfaces
    UDP_PORT = 5005

    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))

    while True:
        data, addr = sock.recvfrom(1024)
        message = data.decode("utf-8")

        # Update the time of the last received UDP command
        global last_command_time
        last_command_time = time.time()

        # Extract the command and file name from the received message
        parts = message.split()
        command = parts[0].lower()
        file_name = parts[1] if len(parts) > 1 else None

        if command == "start":
            # Start playing the WAV file
            print(f"Received command: start {file_name}")
            play_wav(file_name)
        elif command == "stop":
            # Stop playing the WAV file
            print(f"Received command: stop {file_name}")
            stop_all()
        elif command == "stopall":
            # Stop playing all WAV files
            print("Received command: stopall")
            stop_all()

# Scan a directory for WAV files and create objects for each file
def scan_directory(directory):
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".wav"):
                file_path = os.path.join(root, file)
                wave_obj = sa.WaveObject.from_wave_file(file_path)
                file_name = os.path.basename(file_path)
                wav_files[file_name] = wave_obj

# Main function
def main():
    # Scan the directory for WAV files and create objects
    scan_directory(wav_directory)

    # Start the random sound player thread
    random_sound_thread = threading.Thread(target=play_random_sounds)
    random_sound_thread.daemon = True  # Set the thread as a daemon so it doesn't block program exit
    random_sound_thread.start()

    # Start listening for UDP packets
    listen_udp()

# Execute the main function
if __name__ == "__main__":
    main()
