import serial
import time

ser = serial.Serial('/dev/ttyACM0', 115200)

i = 0
j = 0
ID = ""
text_file = open("Pdata.txt", "w")A
while (i < 1000):
	line = ser.readline()
	i = i + 1
	if (line != ""):
		#print line[:-1]         # strip \n
		fields = line[:-1].split('; ');
		ID = fields[0] # + "\n" + ID
		# TIME = int(fields[1])
		# print fields
		print "device ID: ", ID

		# write to file
		if (ID != ""):
			if (ID[0] == "M"):
				text_file.write("{:.3f}".format(float(str(time.time()))) + " " + ID + "\n")
			else:
				if (ID[0] == "P" and len(ID) > 4):
					if (ID[4] == "M"):
						text_file.write("{:.3f}".format(float(str(time.time()))) + " " + ID[4:] + "\n")
		j = j + 1

text_file.close()
print (j)
