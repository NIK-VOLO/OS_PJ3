
all:: test mytest
test: ../my_vm.h
	gcc test.c -g -L../ -lmy_vm -m32 -o test

mytest: ../my_vm.h
	gcc mytest.c -g -L../ -lmy_vm -m32 -o mytest 

clean:
	rm -rf test mytest
