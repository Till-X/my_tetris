CFLAGS= -Wall
cc=g++

main.o:main.cpp
	@$(cc) main.cpp -lncurses -o main
	@./main

run:
	@echo "Thanks for running my game."
	@./main
		
clean:
	-rm main
	
.PHONY:clean
