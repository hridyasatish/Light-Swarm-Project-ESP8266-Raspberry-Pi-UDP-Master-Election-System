import time
import threading
import socket
import struct
from gpiozero import LED, Button

# GPIO configuration
LED_PINS = {
    0x01: LED(24),
    0x02: LED(21),
    0x03: LED(20)
}
whiteLED = LED(19)
button = Button(17)

# UDP Multicast config
MCAST_GRP = '239.0.0.1'
MCAST_PORT = 3000

# Socket setup
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(('', MCAST_PORT))
mreq = struct.pack("=4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
s.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
s.settimeout(1)

# Global state
master_device_id = None
listening = False

def handle_button_press():
    global listening
    listening = not listening
    if listening:
        threading.Thread(target=receive_packets, daemon=True).start()

def receive_packets():
    global master_device_id
    while listening:
        try:
            data, _ = s.recvfrom(1024)
            if len(data) >= 14 and data[0] == 0xF0 and data[13] == 0x0F:
                device_id = data[2]
                master_state = data[3]
                if master_state == 1:
                    update_master(device_id)
        except socket.timeout:
            continue

def update_master(device_id):
    global master_device_id
    for led in LED_PINS.values():
        led.off()
    if device_id in LED_PINS:
        LED_PINS[device_id].on()
    master_device_id = device_id

def main():
    button.when_pressed = handle_button_press
    while True:
        time.sleep(0.1)

if __name__ == '__main__':
    main()
