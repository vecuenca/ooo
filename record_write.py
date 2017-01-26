from subprocess import check_output


block_sizes = [2**x for x in range(5, 22)] + [3145728] # max block size is 3MB
print block_sizes
# block_sizes = [100, 1000, 10000, 100000, 1000000, 1500000, 2000000, 2500000, 2800000, 3000000]
total_bytes = [100000000]

file = "/media/vincent/LEXAR/oo"
file = "output.txt"

print "write rate,block size"
for total in total_bytes:
  for block_size in block_sizes:
    out = check_output(["./create_random_file", file, str(total), str(block_size)])
    seconds = float(out) / 1000
    print(str(total / seconds) + "," + str(block_size))