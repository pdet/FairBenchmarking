import os

os.system('g++ disk.cpp -O3 -march=native -lsqlite3 -o disk')
os.system('g++ memory.cpp -O3 -march=native -lsqlite3 -o memory')


for i in range (10):
	os.system('echo 3 pipe/proc/sys/vm/drop_caches')
	os.system('./disk >> disk.txt')

for i in range (10):
	os.system('echo 3 pipe/proc/sys/vm/drop_caches')
	os.system('./disk >> memory.txt')