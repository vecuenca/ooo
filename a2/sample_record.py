file = open('samplerecord.csv', 'w+')

record_contents = ['a', 'b', 'c', 'd', 'e']
for i in range(5):
	row = ''
	for _ in range(100):
		row += record_contents[i] * 10 + ','
	row = row[:-1]
	file.write(row + '\n')
	

