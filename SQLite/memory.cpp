#include <stdio.h>
#include <sqlite3.h> 
#include <stdlib.h>
#include <chrono>
#include <iostream>

static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   
   for(i = 0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   
   printf("\n");
   return 0;
}

int process_ddl_row(void * pData, int nColumns, char **values, char **columns)
{
   if (nColumns != 1)
      return 1; // Error
   sqlite3* db = (sqlite3*)pData;
   sqlite3_exec(db, values[0], NULL, NULL, NULL);
   return 0;
}

int process_dml_row(void *pData, int nColumns, char **values, char **columns)
{
   if (nColumns != 1)
      return 1; // Error
   sqlite3* db = (sqlite3*)pData;
   char *stmt = sqlite3_mprintf("insert into main.%q ""select * from tpch.%q", values[0], values[0]);
   sqlite3_exec(db, stmt, NULL, NULL, NULL);
   sqlite3_free(stmt);     

   return 0;
}

int executeMemoryDB()
{
   sqlite3* memorydb;
   sqlite3* budb;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   const char* data = "Callback function called";
   std::chrono::time_point<std::chrono::system_clock> start, end;

   sqlite3_open(":memory:", &memorydb);

   sqlite3_open("tpch.db", &budb);       
          
   sqlite3_exec(budb, "BEGIN", NULL, NULL, NULL);
   sqlite3_exec(budb, "SELECT sql FROM sqlite_master WHERE sql NOT NULL",&process_ddl_row, memorydb, NULL);
   sqlite3_exec(budb, "COMMIT", NULL, NULL, NULL);
   sqlite3_close(budb);

   sqlite3_exec(memorydb, "ATTACH DATABASE 'tpch.db' as tpch", NULL, NULL, NULL);

   sqlite3_exec(memorydb, "BEGIN", NULL, NULL, NULL);
   sqlite3_exec(memorydb, "SELECT name FROM tpch.sqlite_master WHERE type='table'", &process_dml_row, memorydb, NULL);
   sqlite3_exec(memorydb, "COMMIT", NULL, NULL, NULL);

   sqlite3_exec(memorydb, "DETACH DATABASE tpch", NULL, NULL,NULL);
   /* Create SQL statement */
   sql = "select l_returnflag, l_linestatus, sum(l_quantity) as sum_qty, sum(l_extendedprice) as sum_base_price, sum(l_extendedprice * (1 - l_discount)) as sum_disc_price, sum(l_extendedprice * (1 - l_discount) * (1 + l_tax)) as sum_charge, avg(l_quantity) as avg_qty, avg(l_extendedprice) as avg_price, avg(l_discount) as avg_disc, count(*) as count_order from lineitem where l_shipdate <= '1998-09-02' group by l_returnflag, l_linestatus order by l_returnflag, l_linestatus;";

   /* Execute SQL statement */
   start = std::chrono::system_clock::now();
   rc = sqlite3_exec(memorydb, sql, callback, (void*)data, &zErrMsg);
   end = std::chrono::system_clock::now();
   std::cout << std::chrono::duration<double>(end - start).count() << std::endl;
   
   sqlite3_close(memorydb);
   return 0;
}



int main(int argc, char* argv[]) {
   return executeMemoryDB();
}