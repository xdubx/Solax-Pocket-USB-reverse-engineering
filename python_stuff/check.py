import serial
import time

ser = serial.Serial(port="/dev/ttyUSB0", baudrate=9600, bytesize=8, timeout=2)    #Open named port 
ser.baudrate = 9600 #9600                     #Set baud rate to 9600
                    #Read ten characters from serial port to data
while (True):
   print(ser.read().hex())

ser.close()  


# serialPort = serial.Serial(
#     port="/dev/ttyUSB0", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
# )
# serialString = ""  # Used to hold data coming over UART
# while 1:
#     # Wait until there is data waiting in the serial buffer
#     if serialPort.in_waiting > 0:

#         # Read data out of the buffer until a carraige return / new line is found
#         serialString = serialPort.readline()

#         # Print the contents of the serial data
#         try:
#             print(serialString.hex())
#         except:
#             pass