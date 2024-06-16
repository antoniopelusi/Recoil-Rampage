.SILENT: clean clean_res_h generate_res_h compile run

all: clean clean_res_h generate_res_h compile run

clean:
	echo "|> Clean\n"
ifneq ("$(wildcard ./RecoilRampage)","")
	rm RecoilRampage
endif

clean_res_h:
	echo "|> Clean resources\n"
ifneq ("$(wildcard ./res_h)","")
	rm -rf res_h
endif

generate_res_h:
	echo "|> Convert resources\n"
	./convert_res.sh

compile:
	echo "|> Compile\n"
	gcc src/main.c -o RecoilRampage -Ofast -std=c99 -I./include/ -I./res_h/ -L./lib/ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

run:
	echo "|> Run\n"
	./RecoilRampage