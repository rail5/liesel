all:
	g++ `Magick++-config --cxxflags --cppflags` -O2 -s -Wall -o liesel liesel.cpp /usr/lib/libGraphicsMagick++.a /usr/lib/libGraphicsMagick.a /usr/lib/libpodofo.a -lssl -lcrypto -lfreetype -lfontconfig -ljpeg -lz `Magick++-config --ldflags --libs`
	
nonstatic:
	g++ `Magick++-config --cxxflags --cppflags` -O2 -s -Wall -o liesel liesel.cpp -lpodofo -lfreetype -lfontconfig -ljpeg -lz `Magick++-config --ldflags --libs`
