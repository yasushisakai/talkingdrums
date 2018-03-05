import serial
import sys

# ser = serial.Serial(
#     port='/dev/tty.usbserial-AH069ENA',
#     baudrate=115200,
#     parity=serial.PARITY_ODD,
#     stopbits=serial.STOPBITS_ONE,
#     bytesize=serial.EIGHTBITS
#         )

ser = serial.Serial('/dev/tty.usbserial-AH069E3Y', 115200)

if ser.isOpen() :
    ser.close()

ser.open()

ser.write('200'.encode())

out = ''

while ser.inWaiting() > 0:
    out += ser.read(40)

if out != '' :
    print(">>" + out)


