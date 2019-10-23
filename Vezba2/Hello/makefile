#Hello make
sources=$(wildcard *.c)
obj=$(sources:.c=.o)

all:hello
hello : $(obj)
	gcc -o $@ $^
%.o : %.c
	gcc -c $^
.PHONY : clean
clean : 
	rm -f hello hello.o
