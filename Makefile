FLAG=-std=c++17 -lpthread -lstdc++fs -w
main:main.cc bundle.cpp
	g++ -o $@ $^ $(FLAG)

.PHONY:
clean:
	rm -rf main
