import binascii
myfile = open("pocket.usb", "r")

for line in myfile:
    a=line[9:-1]
    index = 0
    hexgroup = ""
    for char in a:
        if index <= 1:
            hexgroup = hexgroup + char
            index = index + 1
            if index == 2:
                index = 0
                string = hexgroup.encode("ASCII").hex()
                print(string)
                hexgroup = ""
    #print(a)

myfile.close()