#!/bin/bash

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
rm -rf lineitem.tbl
ln -s tpch-dbgen/lineitem.tbl

# Run
expl_comp_strat/q1 > ../results/hand-sf1-q1.tsv
expl_comp_strat/q1 1 > ../results/hand-sf1-q1-ovflow-chk-sf1.tsv
expl_comp_strat/q1 1 2 > ../results/hand-sf1-q1-ovflow-prev-sf100.tsv
expl_comp_strat/q1 1 2 3 > ../results/hand-sf1-q1-ovflow-chk-sf100.tsv