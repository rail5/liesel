#include "liesel.h"

using namespace std;

bool Liesel::Book::load_document(const string &input, bool pdfstdin, bool speak) {	
	if (!pdfstdin) {
		if (!file_exists(input.c_str())) {
			if (speak) {
			cerr << "Error: File '" << input << "' not found" << endl;
			}
			return false;
		}
		if (!has_ending(input, ".pdf")) {
			if (speak) {
				cerr << "Error: At this stage, Liesel only supports PDFs\nPlease look for a future version to support other formats" << endl;
			}
			return false;
		}
		document = poppler::document::load_from_file(input.c_str());
					
	} else {
		document = poppler::document::load_from_raw_data(input.c_str(), input.size());
	}
	loaded = true;
	return true;
}
		
void Liesel::Book::count_pages(bool verbose) {
	if (loaded) {
		pagecount = document->pages();
		if (verbose) {
			cout << "Pages counted: " << pagecount << endl;
		}
	}
}
		
bool Liesel::Book::set_pages(bool rangeflag, string rangevalue) {
	selectedpages.clear();
	
	if (rangeflag) {
		vector<string> multiranges = explode(rangevalue, ',');
		
		for (long unsigned int i=0;i<multiranges.size();i++) {
			vector<string> singlerange = explode(multiranges[i], '-');
			if (singlerange.size() > 2) {
				cerr << "Error: Invalid range '" << multiranges[i] << "'" << endl;
				return false;
			}
			
			if (singlerange.size() > 1) {
				if (!is_number(singlerange[0]) || !is_number(singlerange[1])) {
					cerr << "Error: Invalid (non-numeric) range '" << multiranges[i] << "'" << endl;
					return false;
				}
				
				int startpage = stoi(singlerange[0]);
				int endpage = stoi(singlerange[1]);
				
				if (startpage == endpage || min(startpage, endpage) == 0) {
					cerr << "Error: Invalid range '" << multiranges[i] << "'" << endl;
					return false;
				}
			
				if (max(startpage, endpage) > pagecount) {
					cerr << "Error: Given range value '" << multiranges[i] << "' out of range for supplied PDF" << endl;
					return false;
				}
				
				for (int x=startpage-1;x>=endpage-1;x--) {
					// Descending ranges (ie, -r 10-1)
					selectedpages.push_back(x);
				}
			
				for (int x=startpage-1;x<endpage;x++) {
					// Ascending (normal) ranges (ie, -r 1-10)
					selectedpages.push_back(x);
				}
			} else if (singlerange.size() == 1) {
				if (!is_number(singlerange[0])) {
					cerr << "Error: Invalid (non-numeric) range '" << multiranges[i] << "'" << endl;
					return false;
				}
				
				int singlepagerange = stoi(singlerange[0]);
				
				if (singlepagerange > pagecount) {
					cerr << "Error: Page '" << singlepagerange << "' out of range for supplied PDF" << endl;
					return false;
				}
				
				if (singlepagerange == 0) {
					cerr << "Error: Invalid page '0'" << endl;
					return false;
				}
				
				selectedpages.push_back(singlepagerange-1);
			} else {
				// User inputted two commas by mistake (-r 1-20,21-40,,,,,,44-45)
				continue;
			}
		}
	} else {
		// Printing entire PDF
		for (int i=0;i<pagecount;i++) {
			selectedpages.push_back(i);
		}
	}
	return true;
}

void Liesel::Book::calculate_segments() {
	int pgcount = selectedpages.size();
	
	if (printjob.segsize > pgcount || printjob.segsize == 0) {
		printjob.segsize = pgcount;
	}
	
	double rdsegcount = (double)pgcount/printjob.segsize;
	printjob.segcount = ceil(rdsegcount);
	
	printjob.finalsegsize = printjob.segsize;
	
	if (pgcount % printjob.segsize != 0) {
		printjob.finalsegsize = pgcount % printjob.segsize;
	}
	
	if (printjob.finalsegsize < printjob.minsize && printjob.segcount > 1) {
		printjob.segcount = printjob.segcount - 1;
		printjob.finalsegsize = (pgcount % printjob.segsize) + printjob.segsize;
	}
	
	printjob.thisseg = 1;
	printjob.startfrom = 0;
	
	/* Keep outfile sanity checks in liesel.cpp */
	
	/* Same goes for checkflag */
}

void Liesel::Book::run_job(bool verbose, bool bookthief, bool pdfstdout) {
	while (printjob.thisseg < printjob.segcount) {
		printjob.startfrom = printjob.segsize*(printjob.thisseg-1);
		
		string newname = printjob.outfilename.substr(0, printjob.outfilename.size()-4) + "-part" + to_string(printjob.thisseg) + ".pdf";
		
		printjob.endat = printjob.startfrom + printjob.segsize;
		
		load_pages(verbose, bookthief);
		make_booklet(verbose, bookthief);
		rescale(verbose, bookthief);
		
		pages.clear();
		if (!pdfstdout) {
			if (verbose) {
				cout << endl << "Writing to file..." << endl;
			}
		
			lhdoc.lhdoc_from_image_vector(booklet);
			lhdoc.write_to_file(newname);
			
			double dpercentdone = (double)printjob.thisseg/printjob.segcount;
			int percentdone = floor(dpercentdone * 100);
			
			if (bookthief) {
				cout << percentdone << "%" << endl;
			}
		} else {
			lhdoc.lhdoc_from_image_vector(booklet);
			lhdoc.write_to_stdout();
		}
			
		
		booklet.clear();
		printjob.thisseg = printjob.thisseg + 1;
	}
	
	printjob.startfrom = printjob.segsize*(printjob.thisseg-1);
	
	string appendtofname = ".pdf";
	
	if (printjob.segcount > 1) {
		appendtofname = "-part" + to_string(printjob.thisseg) + ".pdf";
	}
	
	string newname = printjob.outfilename;
	
	if (!printjob.previewonly) {
		newname = printjob.outfilename.substr(0, printjob.outfilename.size()-4) + appendtofname;
	}

	printjob.endat = printjob.startfrom + printjob.finalsegsize;
	
	load_pages(verbose, bookthief);
	make_booklet(verbose, bookthief);
	rescale(verbose, bookthief);
	
	if (!pdfstdout) {
		if (verbose) {
			cout << endl << "Writing to file..." << endl;
		}
		
		if (printjob.previewonly) {
			writeImages(booklet.begin(), booklet.end(), newname); // Use GraphicsMagick for 'export preview JPEG'
			return;
		}
		
		lhdoc.lhdoc_from_image_vector(booklet);
		lhdoc.write_to_file(newname);
		
		if (bookthief) {
			cout << "100%" << endl;
		}
		cout << "Done!" << endl;
	} else {
		lhdoc.lhdoc_from_image_vector(booklet);
		lhdoc.write_to_stdout();
	}
}

void Liesel::Book::display_progress(int progress, int stage) {
	int number_of_stages = 2; // "Rescaling" is made obsolete with this change
	int progcounter = (progress / number_of_stages) + ((stage - 1) * (100 / number_of_stages)); // Calculate initial value
	
	progcounter = (progcounter / printjob.segcount) + ((printjob.thisseg - 1) * (100 / printjob.segcount)); // Divide it, based on the number of segments
	
	bool subtractor = progcounter; // Subtract 0 from 0, 1 from 1,2,3,4,etc
	
	cout << progcounter-subtractor << "%" << endl;
}

void Liesel::Book::load_pages(bool verbose, bool bookthief) {
	if (verbose) {
		cout << "Ready to load PDF pages" << endl;
	}
	
	int vectorindex = 0;
	int y = 0;
	
	for (int i=printjob.startfrom;i<printjob.endat;i++) {
		poppler::page_renderer imagizer;
		imagizer.set_image_format(poppler::image::format_argb32); // Format set to ARGB32 so that the render can be transferred to the Magick++ Image class
		
		poppler::image loadedpage = imagizer.render_page(document->create_page(selectedpages[i]), properties.quality, properties.quality); // Render page as image
		
		Magick::Image page(loadedpage.width(), loadedpage.height(), "BGRA", Magick::StorageType::CharPixel, loadedpage.data()); // Copy raw image bytes to Magick++ for further processing
		
		page.quality(properties.quality);
		page.resolutionUnits(Magick::PixelsPerInchResolution);
		page.density(Magick::Geometry(properties.quality, properties.quality));
		page.matte(false);
		
		if (properties.grayscale) {
			page.type(Magick::GrayscaleType);
		}
		
		if (properties.alterthreshold) {
			page.threshold(properties.threshold);
		}
		
		if (properties.dividepages) {
			size_t halfwidth = page.columns() / 2;
			size_t dividedheight = page.rows();
			
			Magick::Geometry lefthalf = Magick::Geometry(halfwidth, dividedheight, 0, 0);
			Magick::Geometry righthalf = Magick::Geometry(halfwidth, dividedheight, halfwidth, 0);
			
			Magick::Image newpage = page;
			newpage.crop(lefthalf);
			page.crop(righthalf);
			
			pages.push_back(newpage); // Add the left half (newpage), page becomes the right half
		}
		
		pages.push_back(page);
		
		if (properties.cropflag) {
			for (long unsigned int x=0;x<properties.cropvalues.size();x++) {
				if (properties.cropvalues[x] > 100) {
					properties.cropvalues[x] = 100;
				}
			}
			
			int cropleft = ((pages[vectorindex].columns() / 100) * properties.cropvalues[0]) / 2;
			int cropright = ((pages[vectorindex].columns() / 100) * properties.cropvalues[1]) / 2;
			int croptop = ((pages[vectorindex].columns() / 100) * properties.cropvalues[2]) / 2;
			int cropbottom = ((pages[vectorindex].columns() / 100) * properties.cropvalues[3]) / 2;
			
			Magick::Geometry lefthand(pages[vectorindex].columns() - cropleft, pages[vectorindex].rows(), cropleft, 0);
			pages[vectorindex].crop(lefthand);
			
			Magick::Geometry righthand(pages[vectorindex].columns() - cropright, pages[vectorindex].rows(), 0, 0);
			pages[vectorindex].crop(righthand);
			
			Magick::Geometry tophand(pages[vectorindex].columns(), pages[vectorindex].rows() - croptop, 0, croptop);
			pages[vectorindex].crop(tophand);
			
			Magick::Geometry bottomhand(pages[vectorindex].columns(), pages[vectorindex].rows() - cropbottom, 0, 0);
			pages[vectorindex].crop(bottomhand);
			
			if (properties.dividepages) {
				// Crop the righthalf as well
				vectorindex = vectorindex + 1;
				
				Magick::Geometry lefthand(pages[vectorindex].columns() - cropleft, pages[vectorindex].rows(), cropleft, 0);
				pages[vectorindex].crop(lefthand);
				
				Magick::Geometry righthand(pages[vectorindex].columns() - cropright, pages[vectorindex].rows(), 0, 0);
				pages[vectorindex].crop(righthand);
				
				Magick::Geometry tophand(pages[vectorindex].columns(), pages[vectorindex].rows() - croptop, 0, croptop);
				pages[vectorindex].crop(tophand);
				
				Magick::Geometry bottomhand(pages[vectorindex].columns(), pages[vectorindex].rows() - cropbottom, 0, 0);
				pages[vectorindex].crop(bottomhand);
			}
		}
		
		vectorindex = vectorindex + 1;
		y = y + 1;
		
		if (verbose) {
			cout << "Page " << selectedpages[i]+1 << " loaded... " << flush;
		}
		
		if (bookthief) {
			double dprog = (double)(i + 1)/(printjob.endat + 1)*100;
			int prog = floor(dprog);
			display_progress(prog, 1);
		}
	}
	
	size_t width = pages[0].columns();
	size_t height = pages[0].rows();
	
	Magick::Geometry blanksize = Magick::Geometry(width, height);
	
	Magick::Color blankcolor(MaxRGB, MaxRGB, MaxRGB, 0);
	
	Magick::Image blank_image(blanksize, blankcolor);
	
	int current_total = pages.size();
	int number_of_blanks = 0;
	
	if (current_total % 2 != 0 && !printjob.previewonly) {
		number_of_blanks = number_of_blanks + 1;
		current_total = current_total + 1;
		if (verbose) {
			cout << endl << "Blank page added to make an even number" << endl;
		}
	}
	
	if (current_total % 4 != 0 && !printjob.previewonly) {
		number_of_blanks = number_of_blanks + 2;
		// No further need to update current_total
		if (verbose) {
			cout << endl << "Two blank pages added to make divisible by 4" << endl;
		}
	}
	
	for (int appending=0;appending<number_of_blanks;appending++) {
		pages.push_back(blank_image);
	}

	if (verbose) {
		cout << endl << "All pages loaded" << endl;
	}
}

void Liesel::Book::make_booklet(bool verbose, bool bookthief) {

	if (printjob.linear_output) {
		booklet = move(pages);
		return;
	}

	int relevantpagecount = pages.size();
	int relevantpagecountfromzero = relevantpagecount - 1;
	
	int first = 0;
	int second = relevantpagecountfromzero;
	
	int minmargin = properties.widenby;
	
	bool local_widenflag = properties.widenflag;
	int local_widenby = properties.widenby;
	
	Magick::Geometry blanksize = Magick::Geometry(50, 50);
	Magick::Color blankcolor(MaxRGB, MaxRGB, MaxRGB, 0);
	
	while (first <= (relevantpagecountfromzero / 2)) {
		Magick::Image newimg(blanksize, blankcolor);
		newimg.resolutionUnits(Magick::PixelsPerInchResolution);
		newimg.density(Magick::Geometry(properties.quality, properties.quality));
		
		size_t widthone = pages[pages.size()-1].columns();
		size_t heightone = pages[pages.size()-1].rows(); // Width and height of the final page in the pages vector
		
		size_t widthtwo = pages[0].columns();
		size_t heighttwo = pages[0].rows(); // Width and height of the first page in the pages vector
		
		size_t width = max(widthone, widthtwo) * 2; // Pick the wider of the two, and double that space
		size_t height = max(heightone, heighttwo); // Pick the higher of the two heights
		
		if (heightone != heighttwo || widthone != widthtwo) {
			Magick::Geometry matchsize = Magick::Geometry(width/2, height);
			matchsize.aspect(true);
			pages[pages.size()-1].resize(matchsize);
			pages[0].resize(matchsize); // Rescale them to be equal sizes
		}
		
		if (properties.automargin) {
			local_widenflag = true;
			if (properties.maxmargin != 0) {
				double widenconst = (double)properties.maxmargin / (double)(relevantpagecountfromzero / 2);
				local_widenby = first * widenconst;
			} else {
				local_widenby = first * 0.35;
			}
			
			if (local_widenby < minmargin) {
				local_widenby = minmargin;
			}
		}
		
		double dwidenby = ((width / 4) / 100) * local_widenby;
		
		if (local_widenflag) {
			size_t widthwithmargin = (width / 2) - dwidenby;
			Magick::Geometry resizedagain = Magick::Geometry(widthwithmargin, height);
			resizedagain.aspect(true);
			
			pages[pages.size()-1].resize(resizedagain);
			pages[0].resize(resizedagain);
		}
		
		Magick::Geometry newsize = Magick::Geometry(width, height);
		newsize.aspect(true);
		
		newimg.resize(newsize);
		
		if (properties.dividepages && printjob.previewonly) {
			newimg.composite(pages[pages.size()-1], ((width / 2) + (dwidenby * local_widenflag)), 0);
				/*
				Same old trick of using a boolean in math
				If we're not widening, then this goes at position: width/2 + (some amount of widening * 0)
					ie, width/2
				Otherwise,
					width/2 + (some amount of widening * 1)
				*/
			newimg.composite(pages[0], 0, 0);
		} else {
			newimg.composite(pages[pages.size()-1], 0, 0);
			newimg.composite(pages[0], ((width / 2) + (dwidenby * local_widenflag)), 0);
		}
		
		if (!printjob.previewonly || printjob.rescaling) {
			newimg.rotate(90);
		}
		
		booklet.push_back(newimg);
		
		if (verbose) {
			cout << "Combined pages " << second+1 << " and " << first+1 << "... " << flush;
		}
		
		if (bookthief) {
			double dprog = (double)(first + 1)/((relevantpagecountfromzero / 2) + 1)*100;
			int prog = floor(dprog);
			display_progress(prog, 2);
		}
		
		newimg.rotate(-90);
		
		pages.pop_back(); // Delete the last page from the pages vector
		pages.erase(pages.begin()); // Delete the first page from the pages vector
		
		first = first + 1;
		second = second - 1;
		
		if (first <= (relevantpagecountfromzero / 2)) {
			newimg = Magick::Image(blanksize, blankcolor);
			
			newimg.resolutionUnits(Magick::PixelsPerInchResolution);
			newimg.density(Magick::Geometry(properties.quality, properties.quality));
			
			widthone = pages[pages.size()-1].columns();
			heightone = pages[pages.size()-1].rows(); // Width and height of the final page in the pages vector
			
			widthtwo = pages[0].columns();
			heighttwo = pages[0].rows(); // Width and height of the first page in the pages vector
			
			width = max(widthone, widthtwo) * 2; // Pick the wider of the two, and double that space
			height = max(heightone, heighttwo); // Pick the higher of the two heights
			
			if (heightone != heighttwo || widthone != widthtwo) {
				Magick::Geometry matchsize = Magick::Geometry(width/2, height);
				matchsize.aspect(true);
				pages[pages.size()-1].resize(matchsize);
				pages[0].resize(matchsize); // Rescale them to be equal sizes
			}
			
			dwidenby = ((width / 4) / 100) * local_widenby;
			
			if (local_widenflag) {
				size_t widthwithmargin = (width / 2) - dwidenby;
				Magick::Geometry resizedagain = Magick::Geometry(widthwithmargin, height);
				resizedagain.aspect(true);
				pages[pages.size()-1].resize(resizedagain);
				pages[0].resize(resizedagain);
			}
			
			newsize = Magick::Geometry(width, height);
			newsize.aspect(true);
			
			newimg.resize(newsize);
			
			newimg.composite(pages[0], 0, 0);
			newimg.composite(pages[pages.size()-1], ((width / 2) + (dwidenby * local_widenflag)), 0);
			
			if (!printjob.landscapeflip) {
				newimg.rotate(90);
			} else {
				newimg.rotate(-90);
			}
			
			booklet.push_back(newimg);
			
			if (verbose) {
				cout << "Combined pages " << first+1 << " and " << second+1 << "... " << flush;
			}
			
			if (bookthief) {
				double dprog = (double)(first + 1)/((relevantpagecountfromzero / 2) + 1)*100;
				int prog = floor(dprog);
				display_progress(prog, 2);
			}
			
			newimg.rotate(-90);
			
			pages.pop_back();
			pages.erase(pages.begin());
			
			first = first + 1;
			second = second - 1;
		}
		
	}
	
	if (verbose) {
		cout << endl << "New PDF prepared. Ready to write to file..." << endl;
	}
}

void Liesel::Book::rescale(bool verbose, bool bookthief) {
	size_t width = booklet[0].columns();
	size_t height = booklet[0].rows();
	
	width = (properties.quality / 72) * width;
	height = (properties.quality / 72) * height;
	
	if (printjob.rescaling) {
		width = 72*printjob.rescale_width;
		height = 72*printjob.rescale_height;
	}
	
	lhdoc.width = width;
	lhdoc.height = height;
	
	if (printjob.previewonly) {
	
		if (printjob.rescaling) {
	
			Magick::Geometry magick_rescaled(100*printjob.rescale_width, 100*printjob.rescale_height);
			magick_rescaled.aspect(true);
			booklet[0].resize(magick_rescaled);
			booklet[0].rotate(-90);
		}
	}
	
}

Liesel::Book Liesel::load_book(const string &input, bool pdfstdin, bool speak) {
	Book newbook;
	newbook.load_document(input, pdfstdin, speak);
	return newbook;
}
