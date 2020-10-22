GIMP_CFLAGS = $(shell gimptool-2.0 --cflags)
GIMP_LIBS = $(shell gimptool-2.0 --libs)
CFLAGS += $(GIMP_CFLAGS) -Wall -Wno-unused-variable -Wno-pointer-sign -DGIMP_DISABLE_DEPRECATED
LDFLAGS += $(GIMP_LIBS) -lm
SYSTEM_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-admin-bin ./bin/bimp | sed 's/cp \S* \(\S*\)/\1/'|tr -d [\'])
USER_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-bin ./bin/bimp | sed 's/cp \S* \(\S*\)/\1/'|tr -d [\'])

make: 
	which gimptool-2.0 && \
	gcc -o ./bin/bimp -O2 $(CFLAGS) src/*.c src/manipulation-gui/*.c src/images/*.c $(LDFLAGS)

makewin-debug: 
	which gimptool-2.0 && \
	gcc -o ./bin/win32/bimp $(CFLAGS) src/*.c src/manipulation-gui/*.c src/images/*.c $(LDFLAGS)
    
makewin: 
	which gimptool-2.0 && \
	gcc -mwindows -o ./bin/win32/bimp -O2 $(CFLAGS) src/*.c src/manipulation-gui/*.c src/images/*.c $(LDFLAGS)
		
install: 
	mkdir -p "$(USER_INSTALL_DIR)"
	gimptool-2.0 --install-bin ./bin/bimp
	cp -Rf ./bimp-locale/ "$(USER_INSTALL_DIR)"
	
uninstall: 
	gimptool-2.0 --uninstall-bin bimp
	rm -R $(USER_INSTALL_DIR)/bimp-locale

install-admin:
	mkdir -p "$(SYSTEM_INSTALL_DIR)"
	gimptool-2.0 --install-admin-bin ./bin/bimp
	cp -Rf ./bimp-locale/ "$(SYSTEM_INSTALL_DIR)"

uninstall-admin:
	gimptool-2.0 --uninstall-admin-bin bimp
	rm -R $(SYSTEM_INSTALL_DIR)/bimp-locale

clean:
	rm ./bin/bimp
	
all:
	make

