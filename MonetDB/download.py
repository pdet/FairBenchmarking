import os
import urllib2
from configuration import *

print("Downloading MonetDB Source Code...")
for branch in branches:
	monetdb_tarball_url = 'https://www.monetdb.org/downloads/sources/%s.tar.bz2' % branch
	print(monetdb_tarball_url)
	monetdb_code = urllib2.urlopen(monetdb_tarball_url).read()
	f = open('%s/%s.tar.bz2' % (tempdir,branch.split('/')[1]), 'w+')
	f.write(monetdb_code)
	f.close()