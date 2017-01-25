from subprocess import check_output

block_sizes = [1, 10, 100, 1000, 10000, 100000, 1000000, 1500000, 2000000, 2500000, 2800000, 3000000]

chars = 8110984 # num of chars in sample input

file = "/media/vincent/LEXAR/output.txt"
# file = "output.txt"

print file
print "block size,read rate"
for block_size in block_sizes:
  out = check_output(["./get_histogram", file, str(block_size)])
  print str(block_size) + "," +str(out) 