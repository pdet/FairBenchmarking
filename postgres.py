

import os
import sys
import time


NRUNS = 10
POSTGRES_VERSION = '9.6.1'
RESULTCSV = 'postgres.result.csv'
CURRENTDIR = os.getcwd()
PGDATA=os.path.join(CURRENTDIR, 'pgdata')
INSTALLDIR = os.path.join(CURRENTDIR, 'postgresql-build-${VERSION}'.replace('${VERSION}', POSTGRES_VERSION))

def install_postgres():
	os.system('mkdir -p ${BUILD_DIR}'.replace("${BUILD_DIR}", INSTALLDIR))
	if not os.path.isfile('postgresql-${VERSION}.tar.gz'.replace('${VERSION}', POSTGRES_VERSION)):
		if os.system('wget https://ftp.postgresql.org/pub/source/v${VERSION}/postgresql-${VERSION}.tar.gz'.replace("${VERSION}", POSTGRES_VERSION)):
			raise Exception('Failed to download')
	if os.system('tar xvf postgresql-${VERSION}.tar.gz'.replace("${VERSION}", POSTGRES_VERSION)):
		raise Exception("Failed to unzip")
	#os.system('rm postgresql-${VERSION}.tar.gz'.replace("${VERSION}", POSTGRES_VERSION))
	os.chdir('postgresql-${VERSION}'.replace("${VERSION}", POSTGRES_VERSION))
	if os.system('./configure --prefix="${BUILD_DIR}" --disable-debug --disable-cassert CFLAGS="-O3"'.replace("${BUILD_DIR}", INSTALLDIR)):
		raise Exception("Failed to configure")
	if os.system('make'):
		raise Exception("Failed to make")
	if os.system('make install'):
		raise Exception("Failed to install")
	os.chdir(CURRENTDIR)

def cleanup_install():
	os.system('rm postgresql-${VERSION}.tar.gz'.replace("${VERSION}", POSTGRES_VERSION))
	os.system('rm -r postgresql-${VERSION}'.replace("${VERSION}", POSTGRES_VERSION))
	os.system('rm -r ${BUILD_DIR}'.replace("${BUILD_DIR}", INSTALLDIR))

def init_db():
	os.environ['PGDATA'] = PGDATA
	os.system('${BUILD_DIR}/bin/initdb'.replace("${BUILD_DIR}", INSTALLDIR))

def execute_query(query):
	if os.system('${BUILD_DIR}/bin/psql -d postgres -c "${QUERY}" >/dev/null 2>/dev/null'.replace("${BUILD_DIR}", INSTALLDIR).replace("${QUERY}", query)):
		raise Exception("Failed to execute query \"${QUERY}\"".replace("${QUERY}", query))

def execute_file(fpath):
	if os.system('${BUILD_DIR}/bin/psql -d postgres -f ${FILE} > /dev/null'.replace("${BUILD_DIR}", INSTALLDIR).replace("${FILE}", fpath)):
		raise Exception("Failed to execute file \"${FILE}\"".replace("${FILE}", fpath))

def start_database():
	os.environ['PGDATA'] = PGDATA
	os.system("${BUILD_DIR}/bin/pg_ctl -l logfile start".replace("${BUILD_DIR}", INSTALLDIR))
	attempts = 0
	while True:
		try:
			attempts += 1
			if attempts > 100:
				break
			execute_query('SELECT 1')
			break
		except:
			time.sleep(0.1)
			pass

def stop_database():
	os.environ['PGDATA'] = PGDATA
	os.system("${BUILD_DIR}/bin/pg_ctl stop 2>/dev/null".replace("${BUILD_DIR}", INSTALLDIR))

def delete_database():
	os.system('rm -rf "${DBDIR}"'.replace("${DBDIR}", PGDATA))

def load_tpch():
	os.chdir('postgres')
	print("Creating schema")
	execute_file('postgres.schema.sql')
	with open('postgres.load.sql', 'r') as f:
		data = f.read()
		data = data.replace('DIR', os.path.join(CURRENTDIR, 'tpch-dbgen'))
	with open('postgres.load.sql.tmp', 'w') as f:
		f.write(data)
	print("Loading TPCH")
	execute_file('postgres.load.sql.tmp')
	os.system('rm postgres.load.sql.tmp')
	print("Analyzing and building constraints")
	execute_file('postgres.analyze.sql')
	execute_file('postgres.constraints.sql')
	os.chdir(CURRENTDIR)

def generate_tpch(sf=1):
	os.system('rm -rf tpch-dbgen')
	os.system('git clone https://github.com/eyalroz/tpch-dbgen')
	os.chdir('tpch-dbgen')
	os.system('cmake -G "Unix Makefiles" .')
	os.system('make')
	os.system('./dbgen -s ${SCALE_FACTOR}'.replace("${SCALE_FACTOR}", str(sf)))
	# remove trailing |
	for f in os.listdir('.'):
		if len(f) < 4 or f[-4:] != '.tbl': continue
		os.system('rm -f ${FILE}.tmp'.replace("${FILE}", f))
		os.system("sed 's/.$//' ${FILE} > ${FILE}.tmp".replace("${FILE}", f))
		os.system('mv ${FILE}.tmp ${FILE}'.replace("${FILE}", f))
	os.chdir(CURRENTDIR)

def benchmark_query(fpath):
	# hot run
	print("Performing cold run...")
	execute_file(fpath)
	with open(RESULTCSV, 'w+') as f:
		for i in range(NRUNS):
			print("Query %d/%d" % ((i + 1), NRUNS))
			start = time.time()
			execute_file(fpath)
			end = time.time()
			f.write(str(end - start))
			f.write('\n')

def set_configuration(dict):
	os.system('cp ${BUILD_DIR}/share/postgresql.conf.sample ${DBDIR}/postgresql.conf'.replace("${BUILD_DIR}", INSTALLDIR).replace("${DBDIR}", PGDATA))
	with open('${DBDIR}/postgresql.conf'.replace("${DBDIR}", PGDATA), 'a') as f:
		for entry in dict.keys():
			f.write('${PROPERTY} = ${VALUE}\n'.replace('${PROPERTY}', str(entry)).replace('${VALUE}', dict[entry]))



def print_usage():
	print("Usage: python postgres.py [install|generate [sf]|load|benchmark [configuration]]")
	exit(1)

if len(sys.argv) <= 1:
	print_usage()


optimal_configuration = {
	'shared_buffers': '10GB',
	'effective_cache_size': '14GB',
	'maintenance_work_mem': '4GB',
	'work_mem': '10GB',
	'autovacuum': 'off',
	'random_page_cost': '3.5',
	'geqo_threshold': '15',
	'from_collapse_limit': '14',
	'join_collapse_limit': '14',
	'default_statistics_target': '10000',
	'constraint_exclusion': 'on',
	'wal_buffers': '32MB',
	'max_connections': '10',
	'checkpoint_completion_target': '0.9'
}
default_configuration = {
	'shared_buffers': '128kB',
	'effective_cache_size': '1MB',
	'maintenance_work_mem': '1MB',
	'work_mem': '64kB'
}


if sys.argv[1] == 'install':
	print("Installing Postgres")
	stop_database()
	cleanup_install()
	install_postgres()
	set_configuration({})
	print("Finished installing Postgres")
elif sys.argv[1] == 'generate':
	print("Generating TPCH dataset")
	if len(sys.argv) > 2:
		generate_tpch(float(sys.argv[2]))
	else:
		generate_tpch()
	print("Finished generating TPCH dataset")
elif sys.argv[1] == 'load':
	print("Loading TPCH")
	stop_database()
	delete_database()
	init_db()
	start_database()
	load_tpch()
	stop_database()
	print("Finished loading TPCH")
elif sys.argv[1] == 'benchmark':
	print("Running Q1 benchmark")
	if len(sys.argv) <= 2:
		print_usage()
	elif sys.argv[2] == 'optimal':
		set_configuration(optimal_configuration)
	elif sys.argv[2] == 'default':
		set_configuration(default_configuration)
	stop_database()
	start_database()
	benchmark_query('postgres/q01.sql')
	stop_database()
	print("Finished running Q1 benchmark")
elif sys.argv[1] == 'startdb':
	if len(sys.argv) <= 2:
		print_usage()
	elif sys.argv[2] == 'optimal':
		set_configuration(optimal_configuration)
	elif sys.argv[2] == 'default':
		set_configuration(default_configuration)
	stop_database()
	start_database()
else:
	print_usage()

