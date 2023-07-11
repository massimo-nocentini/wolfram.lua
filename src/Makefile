
#-L/usr/local/Wolfram/WolframEngine/13.2/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions/
linux:
	gcc -O3 -fPIC -Wall -shared -o libwolframlua.so libwolframlua.c \
		-I/usr/local/Wolfram/WolframEngine/13.2/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions/ \
		-llua -lWSTP64i4 -lm -lpthread -lrt -lstdc++ -ldl -luuid

install:
	mkdir -p /usr/local/lib/lua/5.4/	# just for ensuring that the dest dir exists
	mkdir -p /usr/local/share/lua/5.4/	# just for ensuring that the dest dir exists
	mv libwolframlua.so /usr/local/lib/lua/5.4/
	cp wolfram.lua /usr/local/share/lua/5.4/

test:
	cd ../test && lua test-liblualua.lua -v #> README
