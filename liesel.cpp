/***************************************************************
 * Name:      liesel
 * Version:   5.0.2
 * Author:    rail5 (andrew@rail5.org)
 * Created:   2021-09-20
 * Copyright: rail5 (https://rail5.org)
 * License:   GNU GPL V3
 **************************************************************/

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <math.h>
#include <sstream>

#include "functions/itoa.h"
#include "functions/explode.h"
#include "functions/has_ending.h"
#include "functions/is_number.h"
#include "functions/file_exists.h"
#include "functions/btfunctions.h"

using namespace std;

bool checkin(char* infile) {
	// Infile sanity checks written as an isolated function to be used in both -i and -p cases
	if (!file_exists(infile)) {
		cout << "Error: File '" << infile << "' not found" << endl;
		return false;
	}
	
	if (!has_ending((string)infile, ".pdf")) {
		cout << "Error: At this stage, Liesel only supports PDFs\nPlease look for a future version to support other formats" << endl;
		return false;
	}
	
	return true;
}

bool checksegout(string outstring, int segments, bool force = false) {

	if (force == true) {
		return true;
	}

	// If we're printing in segments, this checks that output-part2.pdf, output-part3.pdf etc don't already exist, so we don't overwrite them
	int i = 2;
	while (i <= segments) {
		char newname[4096]; // 4096 chars is the standard maximum length of a Unix path, with a 255 char maximum file name
		strcpy(newname, outstring.substr(0, outstring.size()-4).c_str());
		char counter[33];
		itoa(i, counter, 10);
		strcat(newname, "-part");
		strcat(newname, counter);
		strcat(newname, ".pdf");
		if (file_exists(newname)) {
			cout << "Error: File '" << newname << "' already exists!" << endl;
			return false;
		}
		i = i + 1;
	}
	
	return true;
}

int main(int argc,char **argv)
{

	const char* helpstring = "Usage:\nliesel -i input-file.pdf -o output-file.pdf\n\nOptions:\n\n  -i\n    PDF to convert\n\n  -o\n    New file for converted PDF\n\n  -g\n    Convert PDF to greyscale/black and white\n\n  -r\n    Print only within specified range\n    e.g: -r 1-12\n\n  -s\n    Print output PDFs in segments of a given size\n    e.g: -s 40\n      (produces multiple PDFs)\n\n  -f\n    Force overwrites\n      (do not warn about files already existing)\n\n  -v\n    Verbose mode\n\n  -b\n    Always print percentage done\n      (not only when printing in segments)\n\n  -d\n    Specify pixels-per-inch density/quality\n    e.g: -d 100\n      (warning: using extremely large values can crash)\n\n  -t\n    Transform output PDF to print on a specific size paper\n    e.g: -t us-letter\n\n  -p\n    Count pages of input PDF and exit\n\n  -c\n    Check validity of command, and do not execute\n\n  -h\n    Print this help message\n\n  -q\n    Print program info\n\nExample:\n  liesel -i some-book.pdf -g -r 64-77 -f -d 150 -v -b -o ready-to-print.pdf\n  liesel -i some-book.pdf -r 100-300 -s 40 -t a4 -o ready-to-print.pdf\n  liesel -p some-book.pdf\n  liesel -c -i some-book.pdf -o output.pdf\n";
	
	const char* infostring = "BookThief + Liesel Copyright (C) 2021 rail5\nThis program comes with ABSOLUTELY NO WARRANTY.\nThis is free software (GNU GPL V3), and you are welcome to redistribute it under certain conditions.\n\n0. Liesel takes an ordinary PDF and converts it into a booklet-ready PDF to be home-printed\n1. Liesel assumes that the input PDF has pages which are all the same size, and won't work right if its pages are all different sizes\n2. BookThief is a GUI frontend which merely makes calls to Liesel\n3. The source code for both programs is freely available online\n4. Liesel depends on ImageMagick and PoDoFo, two other free (GPL V3-compatible) programs\n";

	bool grayscale = false;
	bool rangeflag = false;
	bool overwrite = false;
	bool checkflag = false;
	bool verbose = false;
	bool bookthief = false;
	bool lastpageblank = false;
	bool flastpageblank = false;
	bool extrablanks = false;
	bool fextrablanks = false;
	
	char *infile = NULL;
	char *outfile = NULL;
	char *rangevalue = NULL;

	int segsize = 0;
	int rangestart = 0;
	int rangeend;
	int rangelength;
	
	int quality = 100;
	
	string rescale = "none";
	const string rescalers[2] = {"us-letter", "a4"};
	
	/************
	grayscale:
		A flag tripped when the user types -g
		If this flag is tripped to return "true," Liesel converts an inputted color PDF to black&white
	
	rangeflag:
		A flag tripped when the user specifies a range with -r
		If this flag is tripped to return "true," Liesel first checks the validity of the range, and then modifies the call to loadpages() with the range-specific info
		
	overwrite:
		A flag tripped when the user types -f
		If this flag is tripped to return "true," Liesel doesn't make a fuss about output files already existing, and just overwrites them
		
	checkflag:
		A flag tripped when the user types -c
		If this flag is tripped to return "true," Liesel checks whether the command is valid, and halts before execution (does NOT execute the command issued). Liesel returns either a specific error, or "OK" (intentionally does not output newline to stdout)
		
	verbose:
		A flag tripped when the user types -v
		This flag is passed to the countpages(), loadpages() and makepamphlet() functions (see functions/btfunctions.h), if true, these functions will write to stdout each step of the way (loaded page 1, loaded page 2, etc)
		Added on 2021-08-27
	
	bookthief:
		Flag tripped if the user types -b
		The -b option is automatically added when a command is called by the BookThief GUI frontend (>= 4.5)
		This is similar to Verbose, except rather than printing "Loading page 3..." it prints, for example, "3%", which BookThief then uses to update a moving progress bar.
		
	lastpageblank / flastpageblank:
		If Liesel receives an odd number of input pages, this flag is tripped, and a blank page is added to the end to make sure that everything prints correctly
		
	extrablanks / fextrablanks:
		Suppose that the input Liesel gets has a page-count which is not divisible by 4
		Well, Liesel increments odd page counts by 1 (see lastpageblank above), but even that doesn't guarantee the number will be divisible by 4 --
			(meaning, 2 pages front & back per sheet of paper)
		Any printer will simply leave the backside of the final sheet blank -- ie, it will put 2 blank pages right in the middle of your booklet
		In order to avoid that, this "extrablanks" flag is tripped when the final page count % 4 != 0, and loadpages() "adds" 2 blank pages to the end (ie, really moves them away from the -middle- of your booklet, by adding them to the end of the output PDF file)
	
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
		
	quality:
		Number inputted via -d option
		Passed to loadpages() function in functions/btfunctions.h
		Determines Magick++ API's "Quality" measure as well as pixel density
		Initialized at 100 (default)
		If input value is less than 75, quality is set to 75
		(Lower values can cause trouble with Magick++)
		Recommended not to use values higher than 300 (could crash, uses up a lot of memory -- if your computer can handle it, then by all means, go for it)
	
	rescale:
		String initialized as "none," if user specifies paper size with -t, replaced with that input
		This string is then passed to mayberescale() in functions/btfunctions.h
	
	rescalers:
		Acceptable values for rescale
		At the moment: us-letter, a4
	************/
	int c;
	
	opterr = 0;
	
	while ((c = getopt(argc, argv, "ci:o:r:s:p:d:t:ghqfvb")) != -1)
		switch(c) {
			case 'c':
				checkflag = true;
				break;
			case 'g':
				grayscale = true;
				break;
			case 'f':
				overwrite = true;
				break;
			case 'p':
			{
				infile = optarg;
				
				if (checkin(infile) != true) {
					return 1;
				}
				
				InitializeMagick(*argv);
	
				string infilestr(infile);
				
				if (checkflag == true) {
					cout << "OK";
					return 0;
				}
				cout << countpages(infilestr, false); // -p intentionally does not output a newline to stdout
				return 0;
				
				break;
			}
			case 'i':
				infile = optarg;
				if (checkin(infile) != true) {
					return 1;
				}
				break;
			case 'o':
				outfile = optarg;
				
				if (!has_ending((string)outfile, ".pdf")) {
					strcat(outfile, ".pdf");
				}
				break;
			case 'r':
				rangeflag = true;
				rangevalue = optarg;
				break;
			case 's':
				if (!is_number(optarg)) {
					cout << "Error: Invalid (non-numeric) segment size '" << optarg << "'" << endl;
					return 1;
				}
				
				segsize = stoi(optarg);
				
				if (segsize < 4) {
					cout << "Error: Segment size cannot be shorter than 4 pages" << endl;
					return 1;
				}
				break;
			case 'd':
				if (!is_number(optarg)) {
					cout << "Error: Invalid (non-numeric) pixel density '" << optarg << "'" << endl;
					return 1;
				}
				
				quality = stoi(optarg);
				
				if (quality < 75) {
					quality = 75;
				}
				break;
			case 't':
				if (find(begin(rescalers), end(rescalers), optarg) != end(rescalers)) {
					rescale = optarg;
				} else {
					cout << "Error: Unrecognized transform '" << optarg << "'" << endl;
					cout << "Valid options:" << endl;
					for (long unsigned int i=0; i<(sizeof(rescalers)/sizeof(*rescalers)); i++) {
						cout << "  " << rescalers[i] << endl;
					}
					return 1;
				}
				break;
			case 'v':
				verbose = true;
				break;
			case 'b':
				bookthief = true;
				break;
			case 'h':
				cout << helpstring;
				return 0;
				break;
			case 'q':
				cout << infostring;
				return 0;
				break;
			case '?':
				if (optopt == 'i' || optopt == 'o' || optopt == 'r' || optopt == 's' || optopt == 'p') {
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
		cout << helpstring;
		return 1;
	}
	
	if (outfile == NULL) {
		cout << helpstring;
		return 1;
	}
	
	if (file_exists(outfile) && overwrite == false) {
		cout << "Error: File '" << outfile << "' already exists!" << endl;
		return 1;
	}
	
	if (rangeflag == true) {
		vector<string> rng = explode(rangevalue, '-');
		
		if (rng.size() < 2) {
			cout << "Error: Invalid range '" << rangevalue << "'" << endl;
			return 1;
		}
		
		if (!is_number(rng[0]) || !is_number(rng[1])) {
			cout << "Error: Invalid (non-numeric) range '" << rangevalue << "'" << endl;
			return 1;
		}
		
		rangestart = stoi(rng[0]);
		rangeend = stoi(rng[1]);
		rangelength = rangeend - rangestart;
		
		if (rangestart >= rangeend || rangestart == 0) {
			cout << "Error: Invalid range '" << rangevalue << "'" << endl;
			return 1;
		} else if (rangelength < 3) {
			cout << "Error: Range cannot be shorter than 4 pages" << endl;
			return 1;
		}
		
		rangestart = rangestart - 1; // preparing for arrays starting at 0 (page counts start at 1)
	}
	
	InitializeMagick(*argv);

	try {
		string infilestr(infile);
		int pagecount = countpages(infilestr, verbose);
		
		if (rangeflag == true) {
			if (rangeend > pagecount) {
				cout << "Error: Given 'range' value out of range for supplied PDF" << endl;
				return 1;
			}
			
			pagecount = rangelength + 1;
			
		}
		
		if (segsize > pagecount || segsize == 0) {
			segsize = pagecount;
		}
		
		double dsegcount = (double)pagecount/segsize;
		int segcount = ceil(dsegcount);
		int finalsegsize = segsize;
		
		if (pagecount % segsize != 0) {
			finalsegsize = pagecount % segsize;
		}
		
		if (finalsegsize < 4) {
			segcount = segcount - 1;
			finalsegsize = (pagecount % segsize) + segsize;
		}
		
		int thisseg = 1;
		
		int firstpage = rangestart;
		
		string outstring = (string)outfile;
		
		if (segcount > 1) {
			if (checksegout(outstring, segcount, overwrite) != true) {
				return 1;
			}
		}
		
		if (checkflag == true) {
			cout << "OK";
			return 0;
		}
		
		int revsegsize = segsize;
		int revfinalsegsize = finalsegsize;
		
		if (segsize % 2 != 0) {
			lastpageblank = true;
			revsegsize = segsize + 1;
		}
		
		if (finalsegsize % 2 != 0) {
			flastpageblank = true;
			revfinalsegsize = finalsegsize + 1;
		}
		
		if (revsegsize % 4 != 0) {
			extrablanks = true;
		}
		
		if (revfinalsegsize % 4 != 0) {
			fextrablanks = true;
		}
		
		if (segcount > 1) {
			vector<Image> loaded = loadpages(segsize, infile, firstpage, grayscale, lastpageblank, extrablanks, verbose, bookthief, segcount, thisseg, quality);		
			vector<Image> pamphlet = mayberescale(makepamphlet(loaded, verbose), rescale, verbose);
			if (verbose == true) {
				cout << endl << "Writing to file..." << endl;
			}
			writeImages(pamphlet.begin(), pamphlet.end(), outfile);
			
			double dpercentdone = (double)thisseg/segcount;
			int percentdone = floor(dpercentdone * 100);
			
			cout << percentdone << "%" << endl;
		
			thisseg = 2;
			
			while (thisseg < segcount && thisseg > 1) {
				firstpage = rangestart + (segsize*(thisseg-1));
				
				char newname[4096];
				strcpy(newname, outstring.substr(0, outstring.size()-4).c_str()); // remove .pdf extension
				
				char thiscounter[33];
				itoa(thisseg, thiscounter, 10);
				
				strcat(newname, "-part");
				strcat(newname, thiscounter);
				strcat(newname, ".pdf"); //ie, ourfile-part2.pdf
				
				loaded = loadpages(segsize, infile, firstpage, grayscale, lastpageblank, extrablanks, verbose, bookthief, segcount, thisseg, quality);
				pamphlet = mayberescale(makepamphlet(loaded, verbose), rescale, verbose);
				if (verbose == true) {
					cout << endl << "Writing to file..." << endl;
				}
				writeImages(pamphlet.begin(), pamphlet.end(), newname);
				
				dpercentdone = (double)thisseg/segcount;
				percentdone = floor(dpercentdone * 100);

				cout << percentdone << "%" << endl;
				
				thisseg = thisseg + 1;
			}
			
			firstpage = rangestart + (segsize*(thisseg-1));
			
			char newname[4096];
			strcpy(newname, outstring.substr(0, outstring.size()-4).c_str());
			char thiscounter[33];
			itoa(thisseg, thiscounter, 10);
			strcat(newname, "-part");
			strcat(newname, thiscounter);
			strcat(newname, ".pdf");

			loaded = loadpages(finalsegsize, infile, firstpage, grayscale, flastpageblank, fextrablanks, verbose, bookthief, segcount, thisseg, quality);
			pamphlet = mayberescale(makepamphlet(loaded, verbose), rescale, verbose);
			if (verbose == true) {
				cout << endl << "Writing to file..." << endl;
			}
			writeImages(pamphlet.begin(), pamphlet.end(), newname);
			
			cout << "100%" << endl;
			
			cout << "Done!" << endl;
			return 0;
		}
		
		vector<Image> loaded = loadpages(finalsegsize, infile, firstpage, grayscale, flastpageblank, fextrablanks, verbose, bookthief, segcount, thisseg, quality);
		vector<Image> pamphlet = mayberescale(makepamphlet(loaded, verbose), rescale, verbose);
		if (verbose == true) {
			cout << endl << "Writing to file..." << endl;
		}
		writeImages(pamphlet.begin(), pamphlet.end(), outfile);
		
		cout << endl << "Done!" << endl;
		return 0;
		
	}
	catch( Exception &error_ )
	{
		cout << "Error:\n" << error_.what() << endl;
		return 1;
	}
	return 0;
}
