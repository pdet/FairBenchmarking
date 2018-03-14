import subprocess
import os
from configuration import *
import time

print("Loading Data...")
for monetdb_install_dir in monetdb_install_dirs:
	monetdb_install_dir = os.path.join(tempdir,monetdb_install_dir)
	client = os.path.join(monetdb_install_dir, 'bin', 'mclient')
	server = os.path.join(monetdb_install_dir, 'bin', 'mserver5')
	if os.system('ls $HOME/.monetdb') != 0:
		f = open('%s/.monetdb' % os.environ['HOME'], 'w+')
		f.write('user=monetdb\npassword=monetdb')
		f.close()
	proc = subprocess.Popen([server, '--daemon=no'])
	while os.system('%s -s "SELECT 1"' % client) != 0:
		time.sleep(1)
	for script in create_scripts:
		script = os.path.join(MAIN_PATH, 'MonetDB', script)
		os.system('%s "%s"' % (client, script))

	proc.kill()