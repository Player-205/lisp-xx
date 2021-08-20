


build:
	@g++ --std=c++20 -o lisp++ main.cpp -Wall -Wextra

run:
	@./lisp++

clean:
	@rm lisp++
