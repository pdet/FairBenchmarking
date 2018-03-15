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

int executeDiskDB(){
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   const char* data = "Callback function called";
   std::chrono::time_point<std::chrono::system_clock> start, end;


   /* Create SQL statement */
   sql = "select l_returnflag, l_linestatus, sum(l_quantity) as sum_qty, sum(l_extendedprice) as sum_base_price, sum(l_extendedprice * (1 - l_discount)) as sum_disc_price, sum(l_extendedprice * (1 - l_discount) * (1 + l_tax)) as sum_charge, avg(l_quantity) as avg_qty, avg(l_extendedprice) as avg_price, avg(l_discount) as avg_disc, count(*) as count_order from lineitem where l_shipdate <= '1998-09-02' group by l_returnflag, l_linestatus order by l_returnflag, l_linestatus;";

   /* Execute SQL statement */
   start = std::chrono::system_clock::now();
   rc = sqlite3_open("tpch.db", &db);
   rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   end = std::chrono::system_clock::now();
   std::cout << std::chrono::duration<double>(end - start).count() << std::endl;
   sqlite3_close(db);
   return 0;
}


int main(int argc, char* argv[]) {
   return executeDiskDB();
}