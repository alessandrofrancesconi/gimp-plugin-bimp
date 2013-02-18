GIMPARGS = $(shell gimptool-2.0 --cflags --libs)
PCREARGS = $(shell pcre-config --cflags --libs)
SYSTEM_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-admin-bin ./bin/bimp | sed 's/cp \S* \(\S*\)/\1/')
USER_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-bin ./bin/bimp | sed 's/cp \S* \(\S*\)/\1/')

make: 
	which gimptool-2.0 && which pcre-config && \
	gcc -o ./bin/bimp -Wall -O2 -Wno-unused-variable -Wno-pointer-sign -Wno-parentheses src/*.c src/manipulation-gui/*.c $(GIMPARGS) $(PCREARGS) -DGIMP_DISABLE_DEPRECATED
	
install: 
	gimptool-2.0 --install-bin ./bin/bimp
	cp -Rf ./bin/win32/bimp-locale/ $(USER_INSTALL_DIR)
	
uninstall: 
	gimptool-2.0 --uninstall-bin bimp
	rm -R $(USER_INSTALL_DIR)/bimp-locale

install-admin:
	gimptool-2.0 --install-admin-bin ./bin/bimp
	cp -Rf ./bin/win32/bimp-locale/ $(SYSTEM_INSTALL_DIR)

uninstall-admin:
	gimptool-2.0 --uninstall-admin-bin bimp
	rm -R $(SYSTEM_INSTALL_DIR)/bimp-locale

clean:
	rm ./bin/bimp

