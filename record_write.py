from subprocess import check_output

byte_sizes = [100, 1000, 10000, 100000, 1000000, 1500000, 2048000, 2500000, 2800000, 3000000]
total_bytes = [100, 1000, 10000, 100000, 1000000, 10000000]

for total in total_bytes:
  for size in byte_sizes:
    out = check_output(["./create_random_file", "output.txt", str(total), str(size)])
    print(out, total, size)