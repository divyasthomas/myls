all: myls

myls: myls.c
	gcc -g -Wall -o myls myls.c

clean:
	$(RM) myls	
	

