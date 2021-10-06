# Goat

**Created esb.c**

	creates a server socket thats listens to incoming client requests(used curl command)
	creates a thread to process the client request
	parses the bmd sent
	validates the parsed bmd
	stores the bmd in the esb_request table(mysql database) if bmd validation successful

**STEP1: Necessary installations:**

	sudo apt update
	sudo apt install build-essential
	sudo apt install libssl-dev
	sudo apt install wget
	sudo apt install curl
	libxml2 for xml parsing: sudo apt-get install libxml2-dev
	Mysql: https://dev.mysql.com/doc/refman/8.0/en/linux-installation.html


**STEP2: Compilation of esb.c(open a command line)**

	gcc -pthread -lxml2 -o esb.o esb.c bmd.c `xml2-config --cflags --libs` `mysql_config --cflags --libs` -lcurl
	output: creates an executable esb.o
	execution: ./esb.o

**STEP3: Sending a client request**

	open another command line
	curl -X POST -d @/full_xmlfilepath http://localhost:8000

