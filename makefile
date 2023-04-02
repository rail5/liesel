CROSS=x86_64-w64-mingw32.static-
CC=$(CROSS)g++
PKG_CONFIG=$(CROSS)pkg-config

MAGICK_CONFIG=Magick++-config
CURRENT_PLATFORM=$(shell uname)

all:
ifeq ($(CURRENT_PLATFORM),Darwin)
	$(eval MAGICK_CONFIG := GraphicsMagick++-config)
endif
	g++ -std=c++17 `$(MAGICK_CONFIG) --cxxflags --cppflags` `pkg-config --cflags poppler` -O2 -s -Wall -o liesel main.cpp -lfreetype -lfontconfig -ljpeg -lz `$(MAGICK_CONFIG) --ldflags --libs` -lpoppler-cpp -lhpdf -lm

openbsd:
	c++ -std=c++17 `$(MAGICK_CONFIG) --cxxflags` `pkg-config --cflags poppler` -I/usr/local/include -O2 -s -Wall -o liesel main.cpp -lfreetype -lfontconfig -ljpeg -lz `$(MAGICK_CONFIG) --ldflags` `pkg-config --libs poppler-cpp` -lpoppler-cpp -lhpdf -lm

windows:
	 $(CC) -Wl,--allow-multiple-definition -std=c++17 `$(PKG_CONFIG) --cflags GraphicsMagick++ poppler` -O2 -s -Wall -o liesel main.cpp -lfreetype -lfontconfig -ljpeg -lz `$(PKG_CONFIG) --libs GraphicsMagick++ poppler-cpp libopenjp2 libharu freetype2`

install:
	install -m 0755 liesel /usr/bin

clean:
	rm -f liesel
	rm -f liesel.exe
