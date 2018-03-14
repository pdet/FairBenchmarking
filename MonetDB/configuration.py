import os
import inspect

MAIN_PATH = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))) # script directory
MAIN_PATH = os.path.split (MAIN_PATH)[0] 

branches = ['Jul2017-SP4/MonetDB-11.27.13','archive/MonetDB-11.13.9']
tempdir = os.path.join(MAIN_PATH, 'tmp')
parameters = '--enable-optimize --disable-debug --disable-assert --with-rubygem-dir=/export/scratch1/home/holanda/.gem/ruby --disable-jaql'
debug_build_parameters = '--enable-debug --disable-optimize --enable-assert'
create_scripts = ['schema.sql','load.sql','constraints.sql']
query_scripts = ['tpcq1.sql']

monetdb_install_dirs = ['MonetDB-11.27.13Install','MonetDB-11.27.13Installdebug','MonetDB-11.13.9Install'] 

NRUNS = 10