all:
		g++ -Ofast -std=c++11 -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -Wall -Irfc24 -lrf24-bcm -lpthread server.cpp -o server
clean:
		rm -f server
