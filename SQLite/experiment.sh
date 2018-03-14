#!/bin/bash

REPS="1 2 3 4 5 6 7 8 9 10"

for rep in $REPS; do
        START=$(date +%s.%N)
        cat ../1.sql | ../sqlite-autoconf-3220000/sqlite3 "$db" > /dev/null
        END=$(date +%s.%N)
        DIFF=$(echo "$END - $START" | bc)
        echo $DIFF
done

for rep in $REPS; do
	START=$(date +%s.%N)
        cat ../9.sql | ../sqlite-autoconf-3220000/sqlite3 "$db" > /dev/null
	END=$(date +%s.%N)
	DIFF=$(echo "$END - $START" | bc)
	echo $DIFF
done

for rep in $REPS; do
        START=$(date +%s.%N)
        cat ../9x.sql | ../sqlite-autoconf-3220000/sqlite3 "$db" > /dev/null
        END=$(date +%s.%N)
        DIFF=$(echo "$END - $START" | bc)
        echo $DIFF
done
