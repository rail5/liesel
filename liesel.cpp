/***************************************************************
 * Name:      liesel
 * Version:   2.0
 * Author:    rail5 (andrew@rail5.org)
 * Created:   2021-08-14
 * Copyright: rail5 (https://rail5.org)
 * License:   GNU GPL v3.0
 **************************************************************/

#include <Magick++.h>
#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <podofo/podofo.h>

using namespace std;
using namespace Magick;

char* itoa(int value, char* result, int base) {
        /**
	 * C++ version 0.4 char* style "itoa":
	 * Written by Lukás Chmela
	 * Released under GPLv3.

	 */
	// check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

std::vector<std::string> explode(std::string const & s, char delimiter) {
	std::vector<std::string> result;
	std::istringstream iss(s);
	
	for (std::string token; std::getline(iss, token, delimiter); ) {
		result.push_back(std::move(token));
	}
	
	return result;
}

inline bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}


inline bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}


inline bool file_exists(const char *name) {
	struct stat buffer;
	return (stat (name, &buffer) == 0);
}

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
		char comd[128] = "";
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


int main(int argc,char **argv)
{

	const char* helpstring = "Usage:\nliesel -i input-file.pdf -o output-file.pdf\n\nOptions:\n\n  -i\n    PDF to convert\n\n  -o\n    New file for converted PDF\n\n  -g\n    Convert PDF to greyscale/black and white\n\n  -r\n    Print only within specified range\n    e.g: -r 1-12\n\n  -p\n    Count pages of input PDF and exit\n\n  -h\n    Print this help message\n\nExample:\n  liesel -i some-book.pdf -g -r 64-77 -o ready-to-print.pdf\n  liesel -p some-book.pdf\n";

	bool grayscale = false;
	bool pageflag = false;
	bool rangeflag = false;
	char *infile = NULL;
	char *outfile = NULL;
	char *rangevalue = NULL;
	
	int rangestart = 0;
	int rangeend;
	int rangelength;
	
	
	/************
	grayscale:
		A flag tripped when the user types -g
		If this flag is tripped to return "true," Liesel converts an inputted color PDF to black&white
	
	pageflag:
		A flag tripped when the user types -p
		If this flag is tripped to return "true," Liesel checks the input file (-i) and returns the total number of pages, and then stops executing (ie, doesn't convert anything, just outputs the page count)
		
	rangeflag:
		A flag tripped when the user specifies a range with -r
		If this flag is tripped to return "true," Liesel first checks the validity of the range, and then modifies the call to loadpages() with the range-specific info
	
	infile/outfile:
		Self-explanatory
		infile must be verified to exist, outfile must be verified to not exist
	
	rangevalue:
		a string like this, for example:
			52-62
		If -r is specified, Liesel splits the string into its two numbers (eg, "52" and "62") and only handles that range/subset of pages from the inputted PDF.
		
	rangestart:
		The first number in the "rangevalue" string
		In the above example "52-62", this would be 52
		Initialized at 0 for later convenience in main()
		
	rangeend:
		The second number
		In the above example, this would be 62
		Uninitialized unless "rangeflag" is tripped
	
	rangelength:
		rangeend - rangestart
		In the above example, this would be 10
		Uninitialized unless "rangeflag" is tripped
	************/
	int c;
	
	opterr = 0;
	
	while ((c = getopt(argc, argv, "i:o:r:p:gh")) != -1)
		switch(c) {
			case 'g':
				grayscale = true;
				break;
			case 'p':
				pageflag = true;
				infile = optarg;
				break;
			case 'i':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'r':
				rangeflag = true;
				rangevalue = optarg;
				break;
			case 'h':
				printf("%s", helpstring);
				return 0;
				break;
			case '?':
				if (optopt == 'i' || optopt == 'o' || optopt == 'r') {
					fprintf(stderr, "Option -%c requires an argument\n", optopt);
				} else if (isprint(optopt)) {
					fprintf(stderr, "Unknown option `-%c'\n", optopt);
				} else {
					fprintf(stderr, "Unknown option character `\\x%x'\n", optopt);
				}
			return 1;
			default:
				abort();
			}
			
	if (infile == NULL) {
		std::cout << helpstring;
		return 1;
	}
	
	if (!file_exists(infile)) {
		std::cout << "Error: File '" << infile << "' not found" << endl;
		return 1;
	}
	
	if (!hasEnding((std::string)infile, ".pdf")) {
		std::cout << "Error: At this stage, Liesel only supports PDFs\nPlease look for a future version to support other formats" << endl;
		return 1;
	}
	
	if (pageflag == true) {
	
		InitializeMagick(*argv);
	
		std::string infilestr(infile);

		char counted[33];
		itoa(countpages(infilestr), counted, 10);
		std::cout << counted;
		return 0;
	}
	
	if (outfile == NULL) {
		std::cout << helpstring;
		return 1;
	}
	
	if (file_exists(outfile)) {
		std::cout << "Error: File '" << outfile << "' already exists!" << endl;
		return 1;
	}
	
	if (!hasEnding((std::string)outfile, ".pdf")) {
		strcat(outfile, ".pdf");
	}
	
	if (rangeflag == true) {
		std::vector<std::string> rng = explode(rangevalue, '-');
		
		if (!is_number(rng[0]) || !is_number(rng[1])) {
			std::cout << "Error: Invalid (non-numeric) range '" << rangevalue << "'" << endl;
			return 1;
		}
		
		rangestart = std::stoi(rng[0]);
		rangeend = std::stoi(rng[1]);
		rangelength = rangeend - rangestart;
		
		if (rangestart >= rangeend || rangestart == 0) {
			std::cout << "Error: Invalid range '" << rangevalue << "'" << endl;
			return 1;
		} else if (rangelength < 3) {
			std::cout << "Error: Range cannot be shorter than 4 pages" << endl;
			return 1;
		}
		
		rangestart = rangestart - 1; // preparing for arrays starting at 0
		
	}
		
	InitializeMagick(*argv);

	try {
		std::string infilestr(infile);
		int pagecount = countpages(infilestr);
		
		vector<Image> loaded;
		
		if (rangeflag == true) {
			if (rangeend > pagecount) {
				std::cout << "Error: Given 'range' value out of range for supplied PDF" << endl;
				return 1;
			}
			
			pagecount = rangelength + 1;
			
		}
		
		loaded = loadpages(pagecount, infile, rangestart, grayscale);
				
		std::list<Image> pamphlet = makepamphlet(pagecount, loaded);
		
		writeImages(pamphlet.begin(), pamphlet.end(), outfile);
		
		std::cout << "Done!" << endl;
	}
	catch( Exception &error_ )
	{
		std::cout << "Error:\n" << error_.what() << endl;
		return 1;
	}
	return 0;
}
