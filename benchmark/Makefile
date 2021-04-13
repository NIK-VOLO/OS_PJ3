
all:: test mytest multi_test
test: ../my_vm.h
	gcc test.c -g -L../ -lmy_vm -m32 -o test

mytest: ../my_vm.h
	gcc mytest.c -g -L../ -lmy_vm -m32 -o mytest 

multi_test: ../my_vm.h
	gcc multi_test.c -g -L../ -lmy_vm -lpthread -m32 -o multi_test

clean:
	rm -rf test mytest multi_test
