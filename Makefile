.SILENT: clean_linux clean_windows clean clean_res_h generate_res_h compile_linux compile_windows compile run

all: compile

clean_linux:
	echo "|> Clean Linux\n"
	rm -f RecoilRampage

clean_windows:
	echo "|> Clean Windows\n"
	rm -f RecoilRampage.exe

clean: clean_linux clean_windows

clean_res_h:
	echo "|> Clean resources\n"
	rm -rf res_h

generate_res_h: clean_res_h
	echo "|> Convert resources\n"
	./convert_res.sh

compile_linux: clean_linux
	echo "|> Compile Linux\n"
	./setup.sh
	./build.sh

compile_windows: clean_windows
	echo "|> Compile Windows\n"
	TARGET=Windows_NT ./setup.sh
	TARGET=Windows_NT ./build.sh

compile: compile_linux compile_windows

run:
	echo "|> Run\n"
	./RecoilRampage
