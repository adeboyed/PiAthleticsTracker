all:
		g++ -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -Wall -Irfc24 -lrf24-bcm client.cpp
		g++ -mfpu=vfp -mflot-abi=hard -march=arm6zk -mtune=arm1176jzf -Wall lightgate.cpp
clean:
		rm -f client.o lightgate.o tracker
