
all: LDFLAGS=-pie -L${HDF5_HOME}/lib -L./node_modules/zlib/lib -L./node_modules/urlcpp -L./node_modules/openssl/openssl/lib -lz -lssl -lcrypto /home/roger/NodeProjects/hdf5.µws/node_modules/urlcpp/liburlcpp.so -ldl -pthread -lhdf5 -lhdf5_hl
all: uSockets main
	g++ $(LDFLAGS) -o test/build/main $(wildcard test/build/src/*.o) $(wildcard test/build/src/eventing/*.o)

ssl: openssl

zlib:
	cd node_modules/zlib && cmake -DCMAKE_INSTALL_PREFIX=`pwd` . && $(MAKE) && $(MAKE) test && $(MAKE) install

openssl: CFLAGS=-fPIC -O4
openssl: CPPFLAGS=-fPIC -O4
openssl: LDFLAGS=-fPIC -O4
openssl:
	cd node_modules/openssl && ./config shared --prefix=`pwd`/openssl --openssldir=`pwd`/openssl/openssl && $(MAKE) && $(MAKE) test && $(MAKE) install

urlcpp:
	cd node_modules/urlcpp && yarn install && make

uriparser:
	cd node_modules/uriparser && mkdir -p build && mkdir -p dist && cd build && cmake -DCMAKE_INSTALL_PREFIX=../dist -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DURIPARSER_BUILD_TESTS=OFF -DURIPARSER_BUILD_DOCS=OFF .. && make && make install

uSockets: CFLAGS=-fPIC -std=c11 -I./node_modules/uSockets/src -I./node_modules/zlib/include -I./node_modules/openssl/openssl/include
uSockets: CPPFLAGS=-flto -Ofast -pthread -std=c++17 -fPIC -I./node_modules/uSockets/src -I./node_modules/uWebSockets/src -I./node_modules/zlib/include -I./node_modules/openssl/openssl/include
uSockets: ./node_modules/uSockets/src/socket.c
	mkdir -p test/build/src/eventing
	mkdir -p test/build/src/crypto
	echo `pwd`
	gcc $(CFLAGS) -c -o test/build/src/context.o ./node_modules/uSockets/src/context.c
	gcc $(CFLAGS) -c -o test/build/src/loop.o ./node_modules/uSockets/src/loop.c
	gcc $(CFLAGS) -c -o test/build/src/socket.o ./node_modules/uSockets/src/socket.c
	gcc $(CFLAGS) -c -o test/build/src/ssl.o ./node_modules/uSockets/src/ssl.c
	gcc $(CFLAGS) -c -o test/build/src/eventing/epoll_kqueue.o ./node_modules/uSockets/src/eventing/epoll_kqueue.c

main: CFLAGS=-fPIC -std=c11 -I./node_modules/uSockets/src -I./node_modules/zlib/include -I./node_modules/openssl/openssl/include
main: CXXFLAGS=-flto -Ofast -pthread -std=c++2a -fPIE -I${HDF5_HOME}/include -I./include -I./node_modules/urlcpp -I./node_modules/cxx-mime-types/include -I./node_modules/rapidjson/include -I./node_modules/uWebSockets/examples -I./node_modules/uSockets/src -I./node_modules/uWebSockets/src -I./node_modules/zlib/include -I./node_modules/openssl/openssl/include
main: LDFLAGS=-pie -L${HDF5_HOME}/lib -L./node_modules/urlcpp -lhdf5 -lhdf5_hl
main: test/src/main.cpp
	g++ $(CXXFLAGS) -c -o test/build/src/main.o test/src/main.cpp

clean:
	cd node_modules/zlib && $(MAKE) clean
	cd node_modules/openssl && $(MAKE) clean

