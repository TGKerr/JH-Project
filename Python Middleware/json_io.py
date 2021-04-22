# -*- coding: utf-8 -*-
from __future__ import print_function
import sys
import serial
import requests
import string
from microfs import ls, rm, put, get, get_serial
import random, json

#def input():
def main():
	print("running")
	result = "test"		##Startup
	port = get_serial()	##Find Micro:Bit port
	port.flush()		##Flush the buffer.
	while True:			##Always loop
		sent = port.readline().decode("latin-1")		##Read input from port
		if(len(sent) > 0):								##If data
			sent = sent.rstrip()						##Removes trailing spaces.
			print(len(sent))
			if (len(sent) == 1):						##If not valid continue
				continue
			print("Recieved " + sent + " From Microbit")	##Print what was received.
			print("Recieved Get Request")
			if(sent.startswith("Email")):			##Email, since removed.
				##print(port.write(gmail().encode("ascii")))
				continue
			if(sent.startswith("Time")):			##Handle time request.
				data = requests.get("http://127.0.0.1:9432/time/").content.decode("ascii")
				print(data.encode("utf-8"))
				port.write(data.encode("ascii"))
				continue
			if(sent.startswith("Weather")):			##Handle weather request
				print("Getting Weather")
				data = requests.get("http://127.0.0.1:9432/weather/").content.decode("ascii")
				print(data)
				port.write(data.encode("ascii"))
				continue
			if(sent.startswith("News ")):		##Handle news Request
				sent = sent[5:]

			if sent == "":
				sent = "SendAllNews"

			data = (requests.get("http://127.0.0.1:9432/News/" + sent)).content.decode("utf-8")
			dataAscii = ''.join(filter(lambda x: x in string.printable, data))	##Remove non ascii
			headlines = (dataAscii.split('&')[0])				##Split headlines.
			indiHeadlines = headlines.split("<br>")
			data = ''.join(indiHeadlines)			##Join data back together.
			print(data)
			print(len(indiHeadlines[0]))
			i = len(data)
			print(port.write(indiHeadlines[0][:50].encode("ascii")))		##Send first 50 chars of first headline.
			print("Sent")
			port.flush() 
		


if __name__ == '__main__':
    main()