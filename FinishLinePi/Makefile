all:
		g++ -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -Wall -Irfc24 -lrf24-bcm -lpthread client.cpp lightgate.cpp -o client
clean:
		rm -f client
