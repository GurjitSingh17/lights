import serial

def pack(a,b,c,d):
	i = 24
	fin = 0
	for el in (a,b,c,d):
		# print i
		# print bin(el)
		# print bin(el << i)
		fin = fin | (el<< i)
		i -= 8
	# print format(fin, "02x")
	# print bin(fin)
	return fin
pack(1,2,3,4)

def char_pack(a,b,c,d):
	return b"".join([chr(el) for el in (a,b,c,d)])

def unpack(b):
	i = 24
	l = []
	while i >= 0:
		l.append((b >> i) & 0xff)
		i -= 8
	return l

ard = serial.Serial("COM4", 9600)
def setColor(r,g,b):
	ard.write(char_pack(1,r,g,b))

def read4():
	for i in range(4):
		print ard.readline()

import time
time.sleep(4)
print 'a'
setColor(255,0,0)
time.sleep(2)
print 'b'
setColor(0, 0, 155)
time.sleep(3)
print 'c'
setColor(0, 155, 0)
print 'd'
ard.write(char_pack(0,0,0,0))
time.sleep(2)
print 'e'
setColor(255,0,0)
ard.close()

      