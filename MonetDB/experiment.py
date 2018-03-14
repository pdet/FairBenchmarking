import os
from configuration import *
import time
import subprocess

def benchmark_query(client,RESULTCSV):
	for query in query_scripts:
		query = os.path.join(MAIN_PATH, 'MonetDB', query)
		# cold run
		os.system('%s "%s"' % (client, query))
		with open(RESULTCSV, 'w+') as f:
			for i in range(NRUNS):
				print("Query %d/%d" % ((i + 1), NRUNS))
				start = time.time()
				os.system('%s "%s"' % (client, query))
				end = time.time()
				f.write(str(end - start))
				f.write('\n')

print("Querying Data...")
for monetdb_install_dir in monetdb_install_dirs:
	RESULTCSV = os.path.join(MAIN_PATH,monetdb_install_dir)
	monetdb_install_dir = os.path.join(tempdir,monetdb_install_dir)
	client = os.path.join(monetdb_install_dir, 'bin', 'mclient')
	server = os.path.join(monetdb_install_dir, 'bin', 'mserver5')
	proc = subprocess.Popen([server, '--daemon=no'])
	while os.system('%s -s "SELECT 1"' % client) != 0:
		time.sleep(1)
	benchmark_query(client,RESULTCSV)
	proc.kill()