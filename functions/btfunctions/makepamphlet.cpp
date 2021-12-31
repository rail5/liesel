using namespace std;
using namespace Magick;

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
