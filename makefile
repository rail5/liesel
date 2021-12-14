all:
	g++ `Magick++-config --cxxflags --cppflags` `pkg-config --cflags poppler` -O2 -s -Wall -o liesel liesel.cpp -lfreetype -lfontconfig -ljpeg -lz `Magick++-config --ldflags --libs` -lpoppler-cpp
	
install:
	install -m 0755 liesel /usr/bin
