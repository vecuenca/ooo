from random import randint

import string
file = open('samplerecord.csv', 'w+')
for i in range(2000):
	row = ''
	for j in range(100):
		row += string.lowercase[randint(0,25)] * 10 + ','
	row = row[:-1]
	file.write(row + '\n')
