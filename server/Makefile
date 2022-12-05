FLAG=-std=c++2a -lpthread -lstdc++fs -ljsoncpp -I./liba/include -L./liba/lib -lbundle #-w	忽略warning
main:main.cc
	g++ -o $@ $^ $(FLAG) #-g

.PHONY:
clean:
	rm -rf main
