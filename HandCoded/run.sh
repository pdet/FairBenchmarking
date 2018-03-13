#!/bin/bash
# Call like ./run.sh results/hand-sf1-q1.tsv

# TPCH
git clone https://github.com/eyalroz/tpch-dbgen
cd tpch-dbgen
cmake .
make -j4

# generate SF1
./dbgen -s 1 -T L -f

cd ..

# Build
cd expl_comp_strat
cmake .
make -j4
cd ..

# Link tables
ln -s tpch-dbgen/lineitem.tbl

# Run
expl_comp_strat/q1 > $1

