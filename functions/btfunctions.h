#ifndef MAGICK
#define MAGICK
#include <Magick++.h>
#endif

#include <poppler/cpp/poppler-global.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page-renderer.h>


#include <memory>
using namespace std;
using namespace Magick;

int countpages(string infile, bool verbose, bool checkflag) {
	poppler::document* document = poppler::document::load_from_file(infile.c_str());
	
	if (verbose == true && checkflag == false) {
		cout << "Pages counted: " << document->pages() << "\n";
	}
	
	return document->pages();
}

void progresscounter(int prog, int stage, int numstages, int segcount, int thisseg) {
	int progcounter = (prog / numstages) + ((stage - 1) * (100 / numstages)); // calculate initial value
	
	progcounter = (progcounter / segcount) + ((thisseg - 1) * (100 / segcount)); // divide it based on how many segments we're printing
	
	bool subtractor = progcounter;
	
			/*
			We always want to subtract '1' from the progress counter to accommodate the time it takes to write to a file
			ie, 25% in processing, but we're writing 4 segments, so '25%' should be reserved for when that file
			is done being written, so we decrement to '24%.'
			
			However, if the progress counter is currently at '0%', subtracting '1' can cause errors if the result ('-1%')
			is later processed by another program (for example, the BookThief GUI, which updates a graphical progress bar
			from this output)
			
			So, we feed the progress counter into a boolean type
			
			If you feed any number greater than 0 into a boolean type, it returns 1
			If you feed 0 into a boolean type, it returns 0
			
			We then subtract the result from the progress counter (ie, 0 minus 0 or 1,2,3,etc minus 1)
			
			The 2nd law of *nix programming:
			Expect the output of your program to become the input of another, as yet unknown, program. Write programs to
			work together
			*/
	
	cout << progcounter-subtractor << "%" << endl;
}

vector<Image> loadpages(int pgcount, string pdfsource, int startfrom, bool grayscale, bool finalpageblank, bool extrablanks, bool verbose, bool bookthief, int segcount, int thisseg, int quality, int numstages) {

	vector<Image> tocombine;
	
	poppler::document* document = poppler::document::load_from_file(pdfsource.c_str());
	
	int ourpages = startfrom + pgcount;
	
	if (verbose == true) {
		cout << "Ready to load PDF pages\n";
	}
	
	for (int i=startfrom;i<ourpages;i++) {
		poppler::page_renderer imagizer;
		imagizer.set_image_format(poppler::image::format_argb32); // Format set to ARGB32 so that the render can be transferred to the Magick++ Image class
		
		poppler::image loadedpage = imagizer.render_page(document->create_page(i), quality, quality); // Render page as image
		
		Image page(loadedpage.width(), loadedpage.height(), "BGRA", StorageType::CharPixel, loadedpage.data()); // Copy raw pixel data to Magick++ Image class for further processing
		
		// Magick++'s "load" function is SLOW. Replacing it with Poppler's, and copying the raw data over, speeds up full-program execution by an average of 408%
		// In testing, I've even had one or two extreme cases which took 15 minutes using the old method, and now only take about a minute
		
		page.quality(quality);
		page.resolutionUnits(PixelsPerInchResolution);
		page.density(Geometry(quality,quality));
		page.matte(false);
		
		if (grayscale == true) {
				page.type(GrayscaleType);
		}
		
		tocombine.push_back(page);
		
		if (verbose == true) {
			cout << "Page " << i+1 << " loaded... " << flush;
		}
		
		if (bookthief == true) {
			double dprog = (double)(i + 1)/(ourpages + 1)*100;
			int prog = floor(dprog);
			progresscounter(prog, 1, numstages, segcount, thisseg);
		}
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

vector<Image> makepamphlet(vector<Image> &imagelist, bool verbose, bool bookthief, int segcount, int thisseg, int numstages) {

	// We pass &imagelist as a reference so that we can clear its memory progressively as we finish with it, saving on resource usage

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

		
		newimg.composite(imagelist[imagelist.size()-1], 0, 0);
		newimg.composite(imagelist[0], originalwidth, 0);

		newimg.rotate(90);

		recollater.push_back(newimg);
		
		if (verbose == true) {
			cout << "Combined pages " << second+1 << " and " << first+1 << "... " << flush;
		}
		
		if (bookthief == true) {
			double dprog = (double)(first + 1)/((pgcountfromzero / 2) + 1)*100;
			int prog = floor(dprog);
			progresscounter(prog, 2, numstages, segcount, thisseg);
		}
		
		newimg.rotate(-90);
		
		imagelist.pop_back();
		imagelist.erase(imagelist.begin()); // Here we clear the unneeded memory (last and first in the array), and carry on
		
		first = first + 1;
		second = second - 1;
		
		if (first <= (pgcountfromzero / 2)) {
			newimg.composite(imagelist[0], 0, 0);
			newimg.composite(imagelist[imagelist.size()-1], originalwidth, 0);
			
			newimg.rotate(90);

			recollater.push_back(newimg);
			
			if (verbose == true) {
				cout << "Combined pages " << first+1 << " and " << second+1 << "... " << flush;
			}
			
			if (bookthief == true) {
				double dprog = (double)(first + 1)/((pgcountfromzero / 2) + 1)*100;
				int prog = floor(dprog);
				progresscounter(prog, 2, numstages, segcount, thisseg);
			}
		
			newimg.rotate(-90);
			
			imagelist.pop_back();
			imagelist.erase(imagelist.begin());
			
			first = first + 1;
			second = second - 1;
		}
	}
	
	if (verbose == true) {
		cout << "\nNew PDF prepared. Ready to write to file...\n";
	}
	
	return recollater;
	
}

vector<Image> mayberescale(vector<Image> pamphlet, bool rescaling, double outwidth, double outheight, int quality, bool verbose, bool bookthief, int segcount, int thisseg, int numstages) {

	int pagecount = pamphlet.size();
	
	size_t width = pamphlet[0].columns();
	size_t height = pamphlet[0].rows();

	if (rescaling) {
	
		size_t newwidth = width;
		size_t newheight = height;
		
		size_t widthmults[10];
		size_t heightmults[10];
		
		for (int x=0; x<10; x++) {
			widthmults[x] = (x+1)*(quality*outwidth);
			heightmults[x] = (x+1)*(quality*outheight);
		}

		int i = 0;
		while (i < 10) {
			if ((width <= widthmults[i]) || (height <= heightmults[i])) {
				newwidth = widthmults[i];
				newheight = heightmults[i];
				if (verbose == true) {
					cout << endl << "Rescaling to " << newwidth << "x" << newheight << endl;
				}
				break;
			}
					
			if ((width - widthmults[i] >= 200) || (height - heightmults[i] >= 200)) {
				i = i + 1;
			} else {
				newwidth = widthmults[i];
				newheight = heightmults[i];
				if (verbose == true) {
					cout << endl << "Rescaling to " << newwidth << "x" << newheight << endl;
				}
				break;
			}
		}
		
		Geometry newsize = Geometry(newwidth, newheight);
		newsize.aspect(true);
		for (int y=0; y<pagecount; y++) {
			if (verbose == true) {
				cout << "Rescaling page " << y+1 << "... " << flush;
			}
			
			if (bookthief == true) {
				double dprog = (double)(y + 1)/(pagecount + 1)*100;
				int prog = floor(dprog);
				progresscounter(prog, 3, numstages, segcount, thisseg);
			}
			
			pamphlet[y].resize(newsize);
		}
		
	}
	
	return pamphlet;

}

