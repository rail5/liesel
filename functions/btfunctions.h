#ifndef MAGICK
#define MAGICK
#include <Magick++.h>
#endif

#ifndef PODOFO
#define PODOFO
#include <podofo/podofo.h>
#endif

using namespace std;
using namespace Magick;

int countpages(std::string pdfsource) {
	PoDoFo::PdfError::EnableLogging(false);
	PoDoFo::PdfError::EnableDebug(false);
	
	PoDoFo::PdfMemDocument document;
	document.Load(pdfsource.c_str());
	
	return document.GetPageCount();
}

vector<Image> loadpages(int pgcount, char* pdfsource, int startfrom, bool grayscale) {
	Image page;
	vector<Image> tocombine;
	
	page.quality(100);
	page.resolutionUnits(PixelsPerInchResolution);
	page.density(Geometry(100,100));
	page.matte(false);
	
	if (grayscale == true) {
		page.type(GrayscaleType);
	}
	
	int i = startfrom;
	int ourpages = i + pgcount;
	
	while (i < ourpages) {
		char buffer[33];
		char comd[512] = "";
		strcat(comd, pdfsource);
		strcat(comd, "[");
		itoa(i, buffer, 10);
		strcat(comd, buffer);
		strcat(comd, "]");
		
		page.read(comd);
		
		tocombine.push_back(page);
		
		i = i + 1;
	}
	
	return tocombine;
}

std::list<Image> makepamphlet(int pgcount, vector<Image> imagelist) {
	bool finalpageblank = false;
	
	int pgcountfromzero = pgcount - 1;
	
	if (pgcount % 2 != 0) {
		pgcountfromzero = pgcountfromzero + 1;
		finalpageblank = true;
	}
	
	int first = 0;
	int second = pgcountfromzero;
	
	std::list<Image> recollater;
	
	size_t originalwidth = imagelist[0].columns();
	size_t height = imagelist[0].rows();
	size_t width = originalwidth * 2;
		
	Geometry newsize = Geometry(width, height);
	newsize.aspect(true);
	
	Image newimg;
		
	newimg = imagelist[0];
		
	newimg.resize(newsize);
	
	if (finalpageblank == true) {
		Image blank_image(newsize, Color(MaxRGB, MaxRGB, MaxRGB, 0));
		newimg.composite(blank_image, 0, 0);
		newimg.composite(imagelist[first], originalwidth, 0);
		
		newimg.rotate(90);

		recollater.push_back(newimg);
		
		newimg.rotate(-90);
		
		first = first + 1;
		second = second - 1;
		
		if (first <= (pgcountfromzero / 2)) {
			newimg.composite(imagelist[first], 0, 0);
			newimg.composite(imagelist[second], originalwidth, 0);
			
			newimg.rotate(90);

			recollater.push_back(newimg);
		
			newimg.rotate(-90);
			
			first = first + 1;
			second = second - 1;
		}
	}
	
	while (first <= (pgcountfromzero / 2)) {

		
		newimg.composite(imagelist[second], 0, 0);
		newimg.composite(imagelist[first], originalwidth, 0);

		newimg.rotate(90);

		recollater.push_back(newimg);
		
		newimg.rotate(-90);
		
		first = first + 1;
		second = second - 1;
		
		if (first <= (pgcountfromzero / 2)) {
			newimg.composite(imagelist[first], 0, 0);
			newimg.composite(imagelist[second], originalwidth, 0);
			
			newimg.rotate(90);

			recollater.push_back(newimg);
		
			newimg.rotate(-90);
			
			first = first + 1;
			second = second - 1;
		}
	}
	
	return recollater;
	
}

