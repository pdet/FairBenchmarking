python postgres.py install
# SF1
python postgres.py generate 1
python postgres.py load
# optimized
python postgres.py benchmark optimal
mv postgres.result.csv results/postgres.optimal.sf1-q1.tsv
python postgres.py benchmark default
mv postgres.result.csv results/postgres.default.sf1-q1.tsv
# SF0.01
python postgres.py generate 0.01
python postgres.py load
# optimized
python postgres.py benchmark optimal
mv postgres.result.csv results/postgres.optimal.sf0.01-q1.tsv