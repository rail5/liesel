using namespace std;
using namespace Magick;

vector<Image> loadpages(int pgcount, string pdfsource, bool pdfstdin, int startfrom, vector<int> selectedpages, bool grayscale, bool alterthreshold, double threshold, bool cropflag, vector<int> &cropvalues, bool dividepages, bool finalpageblank, bool extrablanks, bool verbose, bool bookthief, int segcount, int thisseg, int quality, int numstages) {

	vector<Image> tocombine;
	
	poppler::document* document;
	if (!pdfstdin) {
		document = poppler::document::load_from_file(pdfsource.c_str());
	} else {
		document = poppler::document::load_from_raw_data(pdfsource.c_str(), pdfsource.size());
	}
	
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
