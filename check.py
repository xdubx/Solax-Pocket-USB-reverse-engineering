import serial
ser = serial.Serial ("/dev/ttyUSB0")    #Open named port 
ser.baudrate = 9600 #9600                     #Set baud rate to 9600
                    #Read ten characters from serial port to data
while (True):
   print(ser.read())
ser.close()  
