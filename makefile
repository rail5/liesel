all:
	g++ `Magick++-config --cxxflags --cppflags` -O2 -s -Wall -o liesel liesel.cpp /usr/lib/libGraphicsMagick++.a /usr/lib/libGraphicsMagick.a ./libpodofo.a -lssl -lcrypto -lfreetype -lfontconfig -ljpeg -lz `Magick++-config --ldflags --libs`
	
nonstatic:
	g++ `Magick++-config --cxxflags --cppflags` -O2 -s -Wall -o liesel liesel.cpp -lpodofo -lfreetype -lfontconfig -ljpeg -lz `Magick++-config --ldflags --libs`
	
install:
	install -m 0755 liesel /usr/bin
