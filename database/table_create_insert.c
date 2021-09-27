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
//     The connection is represented by an object of type MYSQL (called a handle) and must first be initialized by the function mysql_init() before it is used.

// MYSQL *mysql_init(MYSQL *mysql);
  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
  }
// MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag);
  if (mysql_real_connect(con, "localhost", "localuser", "1234",
          "testdb", 0, NULL, 0) == NULL)
  {
      finish_with_error(con);
  }

  if (mysql_query(con, "DROP TABLE IF EXISTS cars")) {
      finish_with_error(con);
  }

  if (mysql_query(con, "CREATE TABLE cars(id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(255), price INT)")) {
      finish_with_error(con);
  }

  if (mysql_query(con, "INSERT INTO cars VALUES(1,'Audi',52642)")) {
      finish_with_error(con);
  }

  if (mysql_query(con, "INSERT INTO cars VALUES(2,'Mercedes',57127)")) {
      finish_with_error(con);
  }

  if (mysql_query(con, "INSERT INTO cars VALUES(3,'Skoda',9000)")) {
      finish_with_error(con);
  }

  if (mysql_query(con, "INSERT INTO cars VALUES(4,'Volvo',29000)")) {
      finish_with_error(con);
  }

  if (mysql_query(con, "INSERT INTO cars VALUES(5,'Bentley',350000)")) {
      finish_with_error(con);
  }

  if (mysql_query(con, "INSERT INTO cars VALUES(6,'Citroen',21000)")) {
      finish_with_error(con);
  }

  if (mysql_query(con, "INSERT INTO cars VALUES(7,'Hummer',41400)")) {
      finish_with_error(con);
  }

  if (mysql_query(con, "INSERT INTO cars VALUES(8,'Volkswagen',21600)")) {
      finish_with_error(con);
  }

  mysql_close(con);
  exit(0);
}