CXX = g++
CXXFLAGS = -std=gnu++20 -Wall -Wextra -pedantic -O2 -s

PKG_CONFIG = pkg-config
MAGICK_CONFIG = Magick++-config

MAGICK_CXXFLAGS = $(shell $(MAGICK_CONFIG) --cxxflags --cppflags)
MAGICK_LDFLAGS = $(shell $(MAGICK_CONFIG) --ldflags --libs)

POPPLER_CXXFLAGS = $(shell $(PKG_CONFIG) --cflags poppler)
POPPLER_LDFLAGS = $(shell $(PKG_CONFIG) --libs poppler)

EXTRA_LIBS = -lfreetype -lfontconfig -ljpeg -lz -lpoppler-cpp -lhpdf -lm

all:
	$(CXX) $(CXXFLAGS) $(MAGICK_CXXFLAGS) $(POPPLER_CXXFLAGS) -o liesel main.cpp $(MAGICK_LDFLAGS) $(POPPLER_LDFLAGS) $(EXTRA_LIBS)

clean:
	rm -f liesel
