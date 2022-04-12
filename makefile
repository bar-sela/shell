
all: server shell

server: server.o 
	gcc -o server server.o 

shell: shell.o
	gcc -o shell shell.o 
server.o: server.c
	gcc -c server.c 
shell.o: shell.c
	gcc -c shell.c

clean:
	rm *.o  
