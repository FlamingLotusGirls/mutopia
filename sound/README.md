# Mutopia Sound Server

The sounds servers are raspberry pi's that listen for udp commands to play wav files. They also play random sounds if they do not receive any commands.

## Building a new server

* Run [Raspberry Pi Imager](https://www.raspberrypi.com/news/raspberry-pi-imager-imaging-utility/)
  * Install Raspberry Pi Other -> Raspberry Pi OS Lite (64-bit)
  * In Settings set hostname to mutopiasnd0 (change the 0 to the appropriate number)
  * Enable ssh and set username/pwd to mutopia/flgflgflg
  * Wrote the SD card and install on Pi

* Copy soundfiles to server
	
	​	`scp -r wav [mutopia@mutopiasnd0.local](mailto:mutopia@mutopiasnd0.local):.`
	
* Log on to new server
	
	​	`ssh mutopiasnd0.local -l mutopia`
	
* Configure static IP for wireless (ethernet should still be DHCP):

	​	`sudo vi /etc/dhcpcd.conf`
	
	Add the following to the end of the file:

```
interface wlan0
static ip_address=192.168.0.80/24 # set to appropriate address
static routers=192.168.0.1
static domain_name_servers=192.168.0.1
```

* Install required packages:
```
sudo apt update
sudo apt install git
sudo apt-get install -y python3-dev libasound2-dev
sudo apt install pip
pip install simpleaudio
```
* Download repo
  
		`git clone https://github.com/FlamingLotusGirls/mutopia`

* Install and start the service:

```
cd ~/mutopia/sound
sudo cp mutopia.service /etc/systemd/system
sudo systemctl enable mutopia.service
sudo systemctl start mutopia.service
sudo systemctl status mutopia.service
```

* Set sound to max on boot:

	`sudo vi /etc/rc.local`

  Add the following line just before the ‘exit 0’:

```
amixer sset PCM 100%
```

## Updating Sounds

Sound files must be wav files, and are located in _/home/mutopia/wav_. They can be updated via scp. Old files will need to be deleted by ssh-ing into the pi. The initial test sounds are all of the format _#.wav_, but they can actually be any filename. The test sounds should be deleted when adding actual sounds.

## Test Program

There is a test program _soundtest.py_. This will listen for keypresses between 1-9 and then send a start command to the server to play the file _N_.wav where _N_ is the key that was pressed. _0_ will send a stopall command and _q_ will quit.

## Usage

The server listens for udp message on port 5005. The following message are accepted:

**start _<wavfile_name>_**
    
**stop _<wavfile_name>_**
    
**stopall**

Multiple sounds can play at the same time, however, if a start command is sent for a sound that is currently playing, it will be ignored.

If no commands are received for 60 seconds, then the script will start randomly playing sounds in the wav directory at random intervals between 15 and 60 seconds. These time values can be configured in the script.
