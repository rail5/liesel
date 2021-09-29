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

vector<Image> loadpages(int pgcount, char* pdfsource, int startfrom, bool grayscale, bool finalpageblank, bool extrablanks, bool verbose, bool bookthief, int segcount, int thisseg, int quality) {
	Image page;
	vector<Image> tocombine;
	
	page.quality(quality);
	page.resolutionUnits(PixelsPerInchResolution);
	page.density(Geometry(quality,quality));
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
			cout << "Page " << i+1 << " loaded... " << flush;
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
			cout << "\nTwo blank pages added to make divisible by 4\n";
		}
	}
	
	if (verbose == true) {
		cout << "\nAll pages loaded\n";
	}
	
	return tocombine;
}

vector<Image> makepamphlet(vector<Image> imagelist, bool verbose) {

	int pgcount = imagelist.size();
	
	int pgcountfromzero = pgcount - 1;
	
	int first = 0;
	int second = pgcountfromzero;
	
	vector<Image> recollater;
	
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
			cout << "Combined pages " << second+1 << " and " << first+1 << "... " << flush;
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
				cout << "Combined pages " << first+1 << " and " << second+1 << "... " << flush;
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

vector<Image> mayberescale(vector<Image> pamphlet, string rescaler, int quality, bool verbose) {

	int pagecount = pamphlet.size();
	
	size_t width = pamphlet[0].columns();
	size_t height = pamphlet[0].rows();

	if (rescaler != "none") {
	
		size_t newwidth = width;
		size_t newheight = height;
		
		size_t widthmults[10];
		size_t heightmults[10];
		
		if (rescaler == "us-letter") {
			//In the case of 100 PPI:
			//{850, 1700, 2550, 3400, 4250, 5100, 5950, 6800, 7650, 8500};
			//{1100, 2200, 3300, 4400, 5500, 6600, 7700, 8800, 9900, 11000};
			
			for (int x=0; x<10; x++) {
				widthmults[x] = (x+1)*(quality*8.5);
				heightmults[x] = (x+1)*(quality*11);
			}
		} else if (rescaler == "a4") {
			//In the case of 100 PPI:
			//{830, 1660, 2490, 3320, 4150, 4980, 5810, 6640, 7470, 8300};
			//{1170, 2340, 3510, 4680, 5850, 7020, 8190, 9360, 10530, 11700};
			
			for (int x=0; x<10; x++) {
				widthmults[x] = (x+1)*(quality*8.3);
				heightmults[x] = (x+1)*(quality*11.7);
			}
		}
		
		int i = 0;
		while (i < (sizeof(widthmults)/sizeof(*widthmults))) {				
			if ((width <= widthmults[i]) || (height <= heightmults[i])) {
				newwidth = widthmults[i];
				newheight = heightmults[i];
				if (verbose == true) {
					cout << endl << "Rescaling to " << newwidth << "x" << newheight << " for " << rescaler << " paper" << endl;
				}
				i = (sizeof(widthmults)/sizeof(*widthmults)) + 1; // sneaky return
			}
					
			if ((width - widthmults[i] >= 200) || (height - heightmults[i] >= 200)) {
				i = i + 1;
			} else {
				newwidth = widthmults[i];
				newheight = heightmults[i];
				if (verbose == true) {
					cout << endl << "Rescaling to " << newwidth << "x" << newheight << " for " << rescaler << " paper" << endl;
				}
				i = (sizeof(widthmults)/sizeof(*widthmults)) + 1; // sneaky return
			}
		}
		
		Geometry newsize = Geometry(newwidth, newheight);
		newsize.aspect(true);
		for (int y=0; y<pagecount; y++) {
			if (verbose == true) {
				cout << "Rescaling page " << y+1 << "... " << flush;
			}
			pamphlet[y].resize(newsize);
		}
		
	}
	
	return pamphlet;

}

