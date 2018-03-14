COPY INTO region   FROM '/export/scratch2/holanda/benchmarking/tpch1/region.tbl'   USING DELIMITERS '|', '\n' LOCKED;
COPY INTO nation   FROM '/export/scratch2/holanda/benchmarking/tpch1/nation.tbl'   USING DELIMITERS '|', '\n' LOCKED;
COPY INTO supplier FROM '/export/scratch2/holanda/benchmarking/tpch1/supplier.tbl' USING DELIMITERS '|', '\n' LOCKED;
COPY INTO customer FROM '/export/scratch2/holanda/benchmarking/tpch1/customer.tbl' USING DELIMITERS '|', '\n' LOCKED;
COPY INTO part     FROM '/export/scratch2/holanda/benchmarking/tpch1/part.tbl'     USING DELIMITERS '|', '\n' LOCKED;
COPY INTO partsupp FROM '/export/scratch2/holanda/benchmarking/tpch1/partsupp.tbl' USING DELIMITERS '|', '\n' LOCKED;
COPY INTO orders   FROM '/export/scratch2/holanda/benchmarking/tpch1/orders.tbl'   USING DELIMITERS '|', '\n' LOCKED;
COPY INTO lineitem FROM '/export/scratch2/holanda/benchmarking/tpch1/lineitem.tbl' USING DELIMITERS '|', '\n' LOCKED;