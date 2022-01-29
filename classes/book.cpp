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
				
				selectedpages.push_back(singlepagerange);
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
			progresscounter(prog, 1, printjob.numstages, printjob.segcount, printjob.thisseg);
		}
	}
	
	if (printjob.finalpageblank || printjob.extrablanks) {
		size_t width = pages[0].columns();
		size_t height = pages[0].rows();
		
		Magick::Geometry blanksize = Magick::Geometry(width, height);
		
		Magick::Color blankcolor(MaxRGB, MaxRGB, MaxRGB, 0);
		
		Magick::Image blank_image(blanksize, blankcolor);
		
		if (printjob.finalpageblank) {
			pages.push_back(blank_image);
			if (verbose) {
				cout << endl << "Blank page added to make an even number" << endl;
			}
		}
		
		if (printjob.extrablanks) {
			pages.push_back(blank_image);
			pages.push_back(blank_image);
			if (verbose) {
				cout << endl << "Two blank pages added to make divisible by 4" << endl;
			}
		}
	}
	
	if (verbose) {
		cout << endl << "All pages loaded" << endl;
	}
}

Liesel::Book Liesel::load_book(const string &input, bool pdfstdin, bool speak) {
	Book newbook;
	newbook.load_document(input, pdfstdin, speak);
	return newbook;
}
