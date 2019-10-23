#Hello make
sources=$(wildcard *.c)
obj=$(sources:.c=.o)

all:hello
hello : $(obj)
	gcc -o $@ $^
%.o:%.c
	gcc -c $^
%.d:%.c
	gcc -MM $^ > $@;


.PHONY : clean
clean : 
	rm -f *.o *.d hello
-include $(sources:.c=.d)

