#ifndef MAGICK
#define MAGICK
#include <Magick++.h>
#endif

#include <podofo/podofo.h>

#include <memory>
using namespace std;
using namespace Magick;

int countpages(string infile, bool verbose) {
	PoDoFo::PdfError::EnableLogging(false);
	PoDoFo::PdfError::EnableDebug(false);
	
	PoDoFo::PdfMemDocument document;
	
	document.Load(infile.c_str());
	
	if (verbose == true) {
		cout << "Pages counted: " << document.GetPageCount() << "\n";
	}
	
	return document.GetPageCount();
}

vector<Image> loadpages(int pgcount, char* pdfsource, int startfrom, bool grayscale, bool finalpageblank, bool extrablanks, bool verbose, bool bookthief, int segcount, int thisseg) {
	Image page;
	vector<Image> tocombine;
	
	page.quality(100);
	page.resolutionUnits(PixelsPerInchResolution);
	page.density(Geometry(100,100));
	page.matte(false);
	
	if (grayscale == true) {
		page.type(GrayscaleType);
	}
	
	if (verbose == true) {
		cout << "Ready to load PDF pages\n";
	}
	
	int i = startfrom;
	int ourpages = i + pgcount;
	
	while (i < ourpages) {
		char buffer[33];
		char comd[4096] = "";
		strcat(comd, pdfsource);
		strcat(comd, "[");
		itoa(i, buffer, 10);
		strcat(comd, buffer);
		strcat(comd, "]");
		
		page.read(comd);
		
		tocombine.push_back(page);
		
		if (verbose == true) {
			cout << "Page " << i << " loaded... " << flush;
		}
		
		if (bookthief == true) {
			double dprog = (double)(i + 1)/(ourpages + 1)*100/segcount*thisseg;
			int prog = floor(dprog);
			cout << prog << "%" << endl;
		}
		
		i = i + 1;
	}
	
	if (finalpageblank == true) {
		size_t width = tocombine[0].columns();
		size_t height = tocombine[0].rows();
		
		Geometry blanksize = Geometry(width, height);
		
		Image blank_image(blanksize, Color(MaxRGB, MaxRGB, MaxRGB, 0));
		
		tocombine.push_back(blank_image);
		
		if (verbose == true) {
			cout << "\nBlank page added to make an even number\n";
		}
	}
	
	if (extrablanks == true) {
		size_t width = tocombine[0].columns();
		size_t height = tocombine[0].rows();
		
		Geometry blanksize = Geometry(width, height);
		
		Image blank_image(blanksize, Color(MaxRGB, MaxRGB, MaxRGB, 0));
		
		tocombine.push_back(blank_image);
		tocombine.push_back(blank_image);
		
		if (verbose == true) {
			cout << "\nTwo extra blanks added to make divisible by 4\n";
		}
	}
	
	if (verbose == true) {
		cout << "\nAll pages loaded\n";
	}
	
	return tocombine;
}

list<Image> makepamphlet(vector<Image> imagelist, bool verbose) {

	int pgcount = imagelist.size();
	
	int pgcountfromzero = pgcount - 1;
	
	int first = 0;
	int second = pgcountfromzero;
	
	list<Image> recollater;
	
	size_t originalwidth = imagelist[0].columns();
	size_t height = imagelist[0].rows();
	size_t width = originalwidth * 2;
		
	Geometry newsize = Geometry(width, height);
	newsize.aspect(true);
	
	Image newimg;
		
	newimg = imagelist[0];
		
	newimg.resize(newsize);
	
	while (first <= (pgcountfromzero / 2)) {

		
		newimg.composite(imagelist[second], 0, 0);
		newimg.composite(imagelist[first], originalwidth, 0);

		newimg.rotate(90);

		recollater.push_back(newimg);
		
		if (verbose == true) {
			cout << "Combined pages " << second << " and " << first << "... " << flush;
		}
		
		newimg.rotate(-90);
		
		first = first + 1;
		second = second - 1;
		
		if (first <= (pgcountfromzero / 2)) {
			newimg.composite(imagelist[first], 0, 0);
			newimg.composite(imagelist[second], originalwidth, 0);
			
			newimg.rotate(90);

			recollater.push_back(newimg);
			
			if (verbose == true) {
				cout << "Combined pages " << first << " and " << second << "... " << flush;
			}
		
			newimg.rotate(-90);
			
			first = first + 1;
			second = second - 1;
		}
	}
	
	if (verbose == true) {
		cout << "\nNew PDF prepared. Ready to write to file...\n";
	}
	
	return recollater;
	
}

