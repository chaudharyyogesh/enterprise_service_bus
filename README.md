# Goat

Created esb.c 
->creates a server socket thats listens to incoming client requests(used curl command)
->creates a thread to process the client request
->parses the bmd sent
->validates the parsed bmd
->stores the bmd in the esb_request table(mysql database) if bmd validation successful

compilation of esb.c(open a command line)
->gcc -pthread -lxml2 -o esb.o esb.c `xml2-config --cflags --libs` `mysql_config --cflags --libs`
output:
->creates an executable exb.o
execution:
->./esb.o

sending a client request
->open another command line
->curl -X POST -d @/xmlfilepath http://localhost:8080

Necessary installations:
->libxml2 for xml parsing: sudo apt-get install libxml2-dev
->Mysql: https://dev.mysql.com/doc/refman/8.0/en/linux-installation.html
