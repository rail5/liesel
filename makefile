CC=$(CROSS)g++
PKG_CONFIG=$(CROSS)pkg-config

all:
	$(CC) -std=c++17 `$(PKG_CONFIG) --cflags GraphicsMagick++ poppler` -O2 -s -Wall -o liesel liesel.cpp -lfreetype -lfontconfig -ljpeg -lz `$(PKG_CONFIG) --libs GraphicsMagick++ poppler-cpp libopenjp2`
	
install:
	install -m 0755 liesel /usr/bin
