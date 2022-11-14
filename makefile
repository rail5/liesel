CROSS=x86_64-w64-mingw32.static-
CC=$(CROSS)g++
PKG_CONFIG=$(CROSS)pkg-config

all:
	g++ -std=c++17 `Magick++-config --cxxflags --cppflags` `pkg-config --cflags poppler` -O2 -s -Wall -o liesel main.cpp -lfreetype -lfontconfig -ljpeg -lz `Magick++-config --ldflags --libs` -lpoppler-cpp -lhpdf -lm

windows:
	 $(CC) -Wl,--allow-multiple-definition -std=c++17 `$(PKG_CONFIG) --cflags GraphicsMagick++ poppler` -O2 -s -Wall -o liesel main.cpp -lfreetype -lfontconfig -ljpeg -lz `$(PKG_CONFIG) --libs GraphicsMagick++ poppler-cpp libopenjp2 libharu freetype2`
	
install:
	install -m 0755 liesel /usr/bin
