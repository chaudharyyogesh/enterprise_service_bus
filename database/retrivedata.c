#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

int main(int argc, char **argv)
{
  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
      fprintf(stderr, "mysql_init() failed\n");
      exit(1);
  }

  if (mysql_real_connect(con, "localhost", "localuser", "1234",
          "testdb", 0, NULL, 0) == NULL)
  {
      finish_with_error(con);
  }

  if (mysql_query(con, "SELECT * FROM cars"))
  {
      finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);
  printf("helo\n");

  if (result == NULL)
  {
      printf("empty");
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);
  printf("numfields:%d\n",num_fields);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      printf("hi");
      for(int i = 0; i < num_fields; i++)
      {
          printf("%s ", row[i] ? row[i] : "NULL");
      }
    // printf("hi%s\n",row[0]);
      printf("\n");
  }
  printf("bye");

  mysql_free_result(result);
  mysql_close(con);

  exit(0);
}