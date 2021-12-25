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

vector<Image> loadpages(int pgcount, string pdfsource, int startfrom, vector<int> selectedpages, bool grayscale, bool alterthreshold, double threshold, bool cropflag, vector<int> &cropvalues, bool dividepages, bool finalpageblank, bool extrablanks, bool verbose, bool bookthief, int segcount, int thisseg, int quality, int numstages) {

	vector<Image> tocombine;
	
	poppler::document* document = poppler::document::load_from_file(pdfsource.c_str());
	
	int ourpages = startfrom + pgcount;
	
	if (verbose == true) {
		cout << "Ready to load PDF pages\n";
	}
	
	int vectorindex = 0;
	int y = 0;
	
	for (int i=startfrom;i<ourpages;i++) {
	
		poppler::page_renderer imagizer;
		imagizer.set_image_format(poppler::image::format_argb32); // Format set to ARGB32 so that the render can be transferred to the Magick++ Image class
		
		poppler::image loadedpage = imagizer.render_page(document->create_page(selectedpages[i]), quality, quality); // Render page as image
		
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
		
		if (alterthreshold == true) {
			page.threshold(threshold);
		}
		
		if (dividepages == true) {
			
			size_t halfwidth = page.columns() / 2;
			size_t dividedheight = page.rows();
			
			Geometry lefthalf = Geometry(halfwidth, dividedheight, 0, 0);
			Geometry righthalf = Geometry(halfwidth, dividedheight, halfwidth, 0);
			
			Image newpage = page;
			newpage.crop(lefthalf);
			page.crop(righthalf);
			
			tocombine.push_back(newpage);
		}
		
		tocombine.push_back(page);
		
		if (cropflag == true) {
			for (long unsigned int x=0;x<cropvalues.size();x++) {
				if (cropvalues[x] > 100) {
					cropvalues[x] = 100;
				}
			}
			
			int cropleft = ((tocombine[vectorindex].columns() / 100) * cropvalues[0]) / 2;
			int cropright = ((tocombine[vectorindex].columns() / 100) * cropvalues[1]) / 2;
			int croptop = ((tocombine[vectorindex].columns() / 100) * cropvalues[2]) / 2;
			int cropbottom = ((tocombine[vectorindex].columns() / 100) * cropvalues[3]) / 2;
			
			Geometry lefthand(tocombine[vectorindex].columns() - cropleft, tocombine[vectorindex].rows(), cropleft, 0);
			tocombine[vectorindex].crop(lefthand);
			Geometry righthand(tocombine[vectorindex].columns() - cropright, tocombine[vectorindex].rows(), 0, 0);
			tocombine[vectorindex].crop(righthand);
			Geometry tophand(tocombine[vectorindex].columns(), tocombine[vectorindex].rows() - croptop, 0, croptop);
			tocombine[vectorindex].crop(tophand);
			Geometry bottomhand(tocombine[vectorindex].columns(), tocombine[vectorindex].rows() - cropbottom, 0, 0);
			tocombine[vectorindex].crop(bottomhand);

			if (dividepages == true) {
				vectorindex = vectorindex + 1;
				Geometry lefthand(tocombine[vectorindex].columns() - cropleft, tocombine[vectorindex].rows(), cropleft, 0);
				tocombine[vectorindex].crop(lefthand);
				Geometry righthand(tocombine[vectorindex].columns() - cropright, tocombine[vectorindex].rows(), 0, 0);
				tocombine[vectorindex].crop(righthand);
				Geometry tophand(tocombine[vectorindex].columns(), tocombine[vectorindex].rows() - croptop, 0, croptop);
				tocombine[vectorindex].crop(tophand);
				Geometry bottomhand(tocombine[vectorindex].columns(), tocombine[vectorindex].rows() - cropbottom, 0, 0);
				tocombine[vectorindex].crop(bottomhand);
			}
		}
		
		vectorindex = vectorindex + 1;
		y = y + 1;
		
		if (verbose == true) {
			cout << "Page " << selectedpages[i]+1 << " loaded... " << flush;
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

vector<Image> makepamphlet(vector<Image> &imagelist, bool verbose, bool bookthief, int segcount, int thisseg, int numstages, bool landscapeflip, int quality, bool widenflag, int widenby, bool previewonly, bool dividepages) {

	// We pass &imagelist as a reference so that we can clear its memory progressively as we finish with it, saving on resource usage

	int pgcount = imagelist.size();
	
	int pgcountfromzero = pgcount - 1;
	
	int first = 0;
	int second = pgcountfromzero;
	
	vector<Image> recollater; // Will be the return vector
	
	
	
	while (first <= (pgcountfromzero / 2)) {

		Image newimg(Geometry(50,50), Color(MaxRGB, MaxRGB, MaxRGB, 0)); // Creating a base image to composite on top of
		newimg.resolutionUnits(PixelsPerInchResolution);
		newimg.density(Geometry(quality,quality));

		size_t widthone = imagelist[imagelist.size()-1].columns();
		size_t heightone = imagelist[imagelist.size()-1].rows();
		size_t widthtwo = imagelist[0].columns();
		size_t heighttwo = imagelist[0].rows();
		
		size_t width = max(widthone,widthtwo) * 2; // pick the wider one, double that space
		size_t height = max(heightone,heighttwo); // higher of the two heights
		
		if (heightone != heighttwo || widthone != widthtwo) {
			Geometry matchsize = Geometry(width/2, height);
			matchsize.aspect(true);
			imagelist[imagelist.size()-1].resize(matchsize);
			imagelist[0].resize(matchsize);
		}
		
		double dwidenby = ((width / 4) / 100) * widenby; // Declare this variable in-scope to be used later
		
		if (widenflag == true) {
			size_t widthwithmargin = (width/2) - dwidenby;
			Geometry resizedagain = Geometry(widthwithmargin, height);
			resizedagain.aspect(true);
			imagelist[imagelist.size()-1].resize(resizedagain);
			imagelist[0].resize(resizedagain);
		}
		
		Geometry newsize = Geometry(width, height);
		newsize.aspect(true);
		
		newimg.resize(newsize);
		
		if (dividepages && previewonly) {
			newimg.composite(imagelist[imagelist.size()-1], ((width/2) + (dwidenby*widenflag)), 0); // again, same old trick of using a boolean in math
														// if we're not widening the center margins at all,
														// then this places the page on the right hand side
														// starting position : width / 2
														// + ((some amount of widening) * (0 or 1 based on whether we're doing that))
														// if we're widening the center by, say, 20
														// then:
														// starting position: width / 2
														// + the amount we're widening by (because we don't want this page's left-hand border
														// to be exactly in the center anymore)
			newimg.composite(imagelist[0], 0, 0);
		} else {
			newimg.composite(imagelist[imagelist.size()-1], 0, 0);
			newimg.composite(imagelist[0], ((width/2) + (dwidenby*widenflag)), 0);
		}
		
		if (previewonly == false || numstages > 2) {
			newimg.rotate(90);
		}

			
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
		
			newimg = Image(Geometry(50,50), Color(MaxRGB, MaxRGB, MaxRGB, 0));
			
			newimg.resolutionUnits(PixelsPerInchResolution);
			newimg.density(Geometry(quality,quality));
		
			widthone = imagelist[imagelist.size()-1].columns();
			heightone = imagelist[imagelist.size()-1].rows();
			widthtwo = imagelist[0].columns();
			heighttwo = imagelist[0].rows();
		
			width = max(widthone,widthtwo) * 2; // pick the wider one, double that space
			height = max(heightone,heighttwo); // higher of the two heights
		
			if (heightone != heighttwo || widthone != widthtwo) {
				Geometry matchsize = Geometry(width/2, height);
				matchsize.aspect(true);
				imagelist[imagelist.size()-1].resize(matchsize);
				imagelist[0].resize(matchsize);
			}
			
			
			dwidenby = ((width / 4) / 100) * widenby;
			if (widenflag == true) {
				size_t widthwithmargin = (width/2) - dwidenby;
				Geometry resizedagain = Geometry(widthwithmargin, height);
				resizedagain.aspect(true);
				imagelist[imagelist.size()-1].resize(resizedagain);
				imagelist[0].resize(resizedagain);
			}
			
			newsize = Geometry(width,height);
			newsize.aspect(true);
			
			newimg.resize(newsize);
		
			newimg.composite(imagelist[0], 0, 0);
			newimg.composite(imagelist[imagelist.size()-1], ((width/2) + (dwidenby*widenflag)), 0);
			
			if (landscapeflip == false) {
				newimg.rotate(90);
			} else {
				newimg.rotate(-90);
			}
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

vector<Image> mayberescale(vector<Image> pamphlet, bool rescaling, double outwidth, double outheight, int quality, bool verbose, bool bookthief, int segcount, int thisseg, int numstages, bool previewonly) {

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
			pamphlet[y].rotate(-90*previewonly); // Rotate -90*0 = 0 if not preview, -90*1 = -90 if preview
		}
		
	}
	
	return pamphlet;

}

