import string
file = open('samplerecord.csv', 'w+')
alphabet = string.lowercase
for i in range(1000):
	row = ''
	for j in range(100):
		row += string.lowercase[j % 26] * 10 + ','
	row = row[:-1]
	file.write(row + '\n')
