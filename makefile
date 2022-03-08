all:
	g++ -std=c++17 `Magick++-config --cxxflags --cppflags` `pkg-config --cflags poppler` -O2 -s -Wall -o liesel main.cpp -lfreetype -lfontconfig -ljpeg -lz `Magick++-config --ldflags --libs` -lpoppler-cpp -lhpdf -lm
	
install:
	install -m 0755 liesel /usr/bin
