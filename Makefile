FLAG=-std=c++17 -lpthread -lstdc++fs -ljsoncpp -I./liba/include -L./liba/lib -lbundle #-w	忽略warning
main:main.cc
	g++ -o $@ $^ $(FLAG) -g

.PHONY:
clean:
	rm -rf main
