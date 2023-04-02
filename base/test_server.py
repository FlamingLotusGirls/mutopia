import socket
import threading
import time

FRAMERATE = 30
UDP_BUFFER_SIZE= 1024 # largest msg we expect to receive
SERVER_PORT = 5431
NODE_PORT = 5432
MSG_PFX = b'MUT'
MAX_NODES = 1024

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

class Node:
    def __init__(self, length):
        self.addr = ''
        self.last_seen = 0
        self.seq_num = 0
        self.chase_state = 0
        self.length = length
        self.strip = []
        for i in range(length):
            self.strip.append([0,0,0])
    def update(self):
        self.seq_num += 1
        data = MSG_PFX + self.seq_num.to_bytes(2,'big') + b'P' + self.length.to_bytes(2,'big')
        for i in range(self.length):
            data += self.strip[i][0].to_bytes(1, 'big') # big or little doesn't matter
            data += self.strip[i][1].to_bytes(1, 'big') # big or little doesn't matter
            data += self.strip[i][2].to_bytes(1, 'big') # big or little doesn't matter
        sock.sendto(data, (self.addr, NODE_PORT))
        #print("sent packet to %s:%d" % (self.addr, NODE_PORT))
    def animate(self):
        #doing a simple chase for now
        for i in range(self.length):
            self.strip[i][0] = 0
            self.strip[i][1] = 0
            self.strip[i][2] = 0
        j = (self.chase_state // 1) % self.length;
        self.strip[j][2] = 255
        self.chase_state += 1
        

# define all the known nodes
nodes = {}
nodes[1] = Node(12); # 12 pixels



def registerMsg(node_id, addr):
    print("received register message")
    try:
        nodes[node_id].addr = addr;
        print("Set node %d to %s" % (node_id, addr))
    except:
        print("ERROR: node %d not initialized" % node_id)


def udp_listen():
    sock.bind(("", SERVER_PORT)) # listen on all interfaces
    while True:
        data, addr = sock.recvfrom(UDP_BUFFER_SIZE)
        if data.startswith(MSG_PFX):
            # we have a valid message

            i = len(MSG_PFX)
            node_id = data[i] * 256 + data[i+1] % 256
            msg_type = chr(data[i+2])
            payload = data[i+3:(len(data))]
            print("received msg type %s from node %d with payload %s" % (msg_type, node_id, payload))

            if msg_type == 'R':
                registerMsg(node_id, addr[0])
            else:
                print("WARNING: unknown message type");

        else:
            print("WARNING: received an invalid msg")


def display():
    while True:
        nodes[1].animate()
        nodes[1].update()

        # cheating for now. This should actually take into account all the render/send time
        time.sleep (1.0 / FRAMERATE)




if __name__ =="__main__":
    # creating thread
    t1 = threading.Thread(target=udp_listen)
    t2 = threading.Thread(target=display)

    # starting thread 1
    t1.start()
    # starting thread 2
    t2.start()

    # wait until thread 1 is completely executed
    t1.join()
    # wait until thread 2 is completely executed
    t2.join()
