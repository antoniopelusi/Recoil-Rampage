.SILENT: clean compile run

all: clean compile run

clean:
	echo "|> Clean\n"
ifneq ("$(wildcard ./RecoilRampage)","")
	rm RecoilRampage
endif

compile:
	echo "|> Compile\n"
	gcc src/main.c -o RecoilRampage -Ofast -std=c99 -I./include/ -L./lib/ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

run:
	echo "|> Run\n"
	./RecoilRampage