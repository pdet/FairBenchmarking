#!/bin/bash

# Setup SQLite
wget https://www.sqlite.org/2018/sqlite-autoconf-3220000.tar.gz
tar -xvf sqlite-autoconf-3220000.tar.gz
cd sqlite-autoconf-3220000
./configure
make -j4
cd ..

# TPCH
git clone https://github.com/eyalroz/tpch-dbgen
cd tpch-dbgen
cmake .
make -j4

# generate SF1
./dbgen -s 1 -f

wget https://raw.githubusercontent.com/lovasoa/TPCH-sqlite/master/sqlite-ddl.sql

# load TPC-H
# based on https://github.com/lovasoa/TPCH-sqlite/blob/master/create_db.sh

TABLES="customer lineitem nation orders partsupp part region supplier"
db="tpch.db"
rm -f "$db"

echo "Creating the database structure..." >&2

# echo "PRAGMA automatic_index = true" | ~/sqlite-autoconf-3220000/sqlite3 "$db"

../sqlite-autoconf-3220000/sqlite3 "$db" < sqlite-ddl.sql

RET_CODE=0
for table in $TABLES; do
        echo "Importing table '$table'..." >&2
        data_file="$table.tbl"
        if [ ! -e "$data_file" ]; then
                echo "'$data_file' does not exist. Skipping..." >&2
                RET_CODE=1
                continue
        fi

        fifo=$(mktemp -u)
        mkfifo $fifo
        sed -e 's/|$//' < "$data_file" > "$fifo" &
        (
                echo ".mode csv";
                echo ".separator |";
                echo -n ".import $fifo ";
                echo $table | tr a-z A-Z;
        ) | ../sqlite-autoconf-3220000/sqlite3 "$db"
        rm $fifo

        if [ $? != 0 ]; then
                echo "Import failed." >&2
                RET_CODE=1
        fi
done


REPS="1 2 3 4 5 6 7 8 9 10"

res="../../results/sqlite-sf1-q9-defaullt-joinorder"
rm -rf "$res"

for rep in $REPS; do
        START=$(date +%s.%N)
        cat ../9.sql | ../sqlite-autoconf-3220000/sqlite3 "$db" > /dev/null
        END=$(date +%s.%N)
        DIFF=$(echo "$END - $START" | bc)
        echo $DIFF >> $res
done


db="tpch.db"
rm -f "$db"

echo "Creating the database structure..." >&2

# echo "PRAGMA automatic_index = true" | ~/sqlite-autoconf-3220000/sqlite3 "$db"

../sqlite-autoconf-3220000/sqlite3 "$db" < sqlite-ddl.sql

RET_CODE=0
for table in $TABLES; do
        echo "Importing table '$table'..." >&2
        data_file="$table.tbl"
        if [ ! -e "$data_file" ]; then
                echo "'$data_file' does not exist. Skipping..." >&2
                RET_CODE=1
                continue
        fi

        fifo=$(mktemp -u)
        mkfifo $fifo
        sed -e 's/|$//' < "$data_file" > "$fifo" &
        (
                echo ".mode csv";
                echo ".separator |";
                echo -n ".import $fifo ";
                echo $table | tr a-z A-Z;
        ) | ../sqlite-autoconf-3220000/sqlite3 "$db"
        rm $fifo

        if [ $? != 0 ]; then
                echo "Import failed." >&2
                RET_CODE=1
        fi
done


res="../../results/sqlite-sf1-q9-hard-joinorder"
rm -rf "$res"

for rep in $REPS; do
        START=$(date +%s.%N)
        cat ../9_samehardcode.sql | ../sqlite-autoconf-3220000/sqlite3 "$db" > /dev/null
        END=$(date +%s.%N)
        DIFF=$(echo "$END - $START" | bc)
        echo $DIFF >> $res
done