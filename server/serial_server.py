import socket
import sys
import serial
from thread import *

ard = serial.Serial("COM3", 9600)
HOST = 'localhost'   
PORT = 8888 
 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print 'Socket created'
 
#Bind socket to local host and port
try:
    s.bind((HOST, PORT))
except socket.error as msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()
     
 
#Start listening on socket
s.listen(10)

write_queue = []
def write_data():
    while True:
        if write_queue:
            ard.write(write_queue.pop())

start_new_thread(write_data, ())

def clientthread(conn):
    while True:
        data = conn.recv(1024)
        write_queue.append(data)
    conn.close()
 

while 1:
    conn, addr = s.accept()
    while True:
        data = conn.recv(1024)
        ard.write(data)
        conn.close()
        break
s.close()