all: kol1
kol1:kol1.o
	gcc -o kol1 kol1.o
kol1.o:kol1.c
	gcc -c kol1.c
.PHONY:clean
clean:
	rm kol1.o kol1
