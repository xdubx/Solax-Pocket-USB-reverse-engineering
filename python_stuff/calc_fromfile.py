#read file in 
#1-2 line shit add


filename = "readFile_1.txt"

with open(filename) as f:
    content = f.readlines()
    lenOfContainer = len(content)
    x = 0
    while x < lenOfContainer-1:
        print( float((int(content[x+1],0) << 8) | (int(content[x],0))) * float(0.1) )
        x += 1
