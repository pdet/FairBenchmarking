# FairBenchmarking

# Compile and Create TPC-H Scale Factor 1 (https://github.com/eyalroz/tpch-dbgen)
cmake -G "Unix Makefiles" . && make && ./dbgen -s 1