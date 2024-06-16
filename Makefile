all: compile run

compile:
	gcc src/main.c -o RecoilRampage -Ofast -std=c99 -I./include/ -L./lib/ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

run:
	./RecoilRampage