GIMPARGS = $(shell gimptool-2.0 --cflags --libs)
PCREARGS = $(shell pcre-config --cflags --libs)
CP_OUTPUT = $(shell gimptool-2.0 --install-bin ./bin/bimp)
SAVE_DIR = $(shell echo $(CP_OUTPUT) | sed 's/cp \S* \(\S*\)/\1/')
make: 
	gcc -o ./bin/bimp -Wall -O2 -Wno-unused-variable -Wno-pointer-sign -Wno-parentheses src/*.c src/manipulation-gui/*.c $(GIMPARGS) $(PCREARGS) -DGIMP_DISABLE_DEPRECATED
	
install: 
	gimptool-2.0 --install-bin ./bin/bimp
	cp -Rf ./bin/win32/bimp-locale/ $(SAVE_DIR)
	
uninstall: 
	gimptool-2.0 --uninstall-bin bimp

clean:
	rm ./bin/bimp

