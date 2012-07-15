GIMPARGS = $(shell gimptool-2.0 --cflags --libs)
PCREARGS = $(shell pcre-config --cflags --libs)
make: 
	gcc -o ./bin/bimp -Wall -O2 -Wno-unused-variable -Wno-pointer-sign -Wno-parentheses src/*.c src/manipulation-gui/*.c $(GIMPARGS) $(PCREARGS) -DGIMP_DISABLE_DEPRECATED
	
install: 
	gimptool-2.0 --install-bin ./bin/bimp
	
uninstall: 
	gimptool-2.0 --uninstall-bin bimp

clean:
	rm ./bin/bimp
