import os
from configuration import *

print("Building MonetDB...")
for branch in branches:
	branch_name = branch.split('/')[1]
	os.chdir(tempdir)

	if os.system('tar xvf %s.tar.bz2' % branch_name) != 0:
		print("Failed to build MonetDB")
		exit()

	os.chdir('%s/%s' % (tempdir, branch_name))
	if os.system('./bootstrap') != 0:
		print("Failed to bootstrap MonetDB")
		exit()

	monetdb_install_dir = os.path.join(tempdir, branch_name+'Install')
	os.system('mkdir %s', monetdb_install_dir)
	if os.system('./configure %s --prefix="%s"' % (parameters, monetdb_install_dir)) != 0:
		print("Failed to configure MonetDB")
		exit()

	if branch_name == 'default':
		if os.system('./configure %s --prefix="%s"' % (debug_build_parameters, monetdb_install_dir+'debugInstall')) != 0:
			print("Failed to configure MonetDB")
			exit()

	if os.system('make install -j4') != 0:
		print("Failed to make install MonetDB")
		exit()
