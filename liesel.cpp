/***************************************************************
 * Name:      liesel
 * Version:   7.1
 * Author:    rail5 (andrew@rail5.org)
 * Created:   2021-12-17
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
#include "functions/is_decimal.h"

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
		string newname = outstring.substr(0, outstring.size()-4) + "-part" + to_string(i) + ".pdf";
		
		if (file_exists(newname)) {
			cout << "Error: File '" << newname << "' already exists!" << endl;
			return false;
		}
		i = i + 1;
	}
	
	return true;
}

bool iswritable(char* path) {
	return (access(path, W_OK) == 0);
}

int main(int argc,char **argv)
{

	const string versionstring = "7.0";

	const string helpstring = "Liesel " + versionstring + "\n\nUsage:\nliesel -i input-file.pdf -o output-file.pdf\n\nOptions:\n\n  -i\n    PDF to convert\n\n  -o\n    New file for converted PDF\n\n  -g\n    Convert PDF to greyscale/black and white\n\n  -r\n    Print only specified range of pages (in the order supplied)\n    e.g: -r 1-12\n    e.g: -r 15-20,25-30\n    e.g: -r 10,9,5,2,1\n    e.g: -r 20-10 (prints backwards)\n\n  -s\n    Print output PDFs in segments of a given size\n    e.g: -s 40\n      (produces multiple PDFs of 40 pages each)\n\n  -m\n    Specify minimum segment size (default is 4)\n    e.g: -m 8\n\n  -f\n    Force overwrites\n      (do not warn about files already existing)\n\n  -v\n    Verbose mode\n\n  -b\n    Show progress (percentage done)\n\n  -d\n    Specify pixels-per-inch density/quality (default is 100)\n    e.g: -d 200\n      (warning: using extremely large values can crash)\n\n  -t\n    Transform output PDF to print on a specific size paper\n      e.g: -t us-letter\n      or: -t 8.5x11\n\n  -l\n    Duplex printer \"landscape\" flipping compatibility\n      (flips every other page)\n\n  -p\n    Count pages of input PDF and exit\n\n  -c\n    Check validity of command, and do not execute\n\n  -h\n    Print this help message\n\n  -q\n    Print program info\n\n  -V\n    Print version number\n\nExample:\n  liesel -i some-book.pdf -g -r 64-77 -f -d 150 -v -b -o ready-to-print.pdf\n  liesel -i some-book.pdf -r 100-300,350-400,1-10 -s 40 -t 8.25x11.75 -m 16 -o ready-to-print.pdf\n  liesel -i some-book.pdf -r 1,5,7,3,1,50 -l -o ready-to-print.pdf\n  liesel -p some-book.pdf\n  liesel -c -i some-book.pdf -o output.pdf\n";
	
	const string infostring = "BookThief + Liesel Copyright (C) 2021 rail5\nThis program comes with ABSOLUTELY NO WARRANTY.\nThis is free software (GNU GPL V3), and you are welcome to redistribute it under certain conditions.\n\n0. Liesel takes an ordinary PDF and converts it into a booklet-ready PDF to be home-printed\n1. BookThief is a GUI front-end which merely makes calls to Liesel\n2. The source code for both programs is freely available online\n3. Liesel depends on ImageMagick and Poppler, two other free (GPL V3-compatible) programs\n";

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
	bool landscapeflip = false;
	
	char *infile = NULL;
	char *outfile = NULL;
	char *rangevalue = NULL;

	int segsize = 0;
	
	vector<int> finalpageselection;
	
	int numstages = 2;
	
	int quality = 100;
	
	int minsize = 4;

	bool rescaling = false;
	double outwidth;
	double outheight;
	map<string, double> widthpresets;
	map<string, double> heightpresets;
	
	widthpresets["us-letter"] = 8.5;
	heightpresets["us-letter"] = 11;
	
	widthpresets["a4"] = 8.3;
	heightpresets["a4"] = 11.7;

	
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
		If -r is specified, Liesel splits the string first by commas (ie, -r 1-4,9-13,7 becomes "1-4" + "9-13" + "7")
		then splits each of those into the page numbers alone, and only handles that range/subset of pages from the inputted PDF.
	
	finalpageselection:
		A vector of all the page numbers, in order, that will be processed
		ie, if the user types -r 1-10
		the vector will be populated with: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
		if the user types -r 1-4,2,7-9,350
		the vector will be populated with: 0, 1, 2, 3, 2, 6, 7, 8, 349
		
		It's worth noting that even though we humans count pages starting from 1, computers count them from 0
		But if you're reading the comments in this source code I probably don't need to tell you that
		
	numstages:
		For the progress counter (-b)
		Normally, there are 2 'stages' to exection: loading pages, and then combining pages
		The progress counter then divides the 'amount done' by the number of stages
			ie, 100% done with page loading, 1st stage out of 2, so we're 50% done
		If the user specifies a transform with -t, that becomes 3 stages instead of 2
		See void progresscounter() in functions/btfunctions.h for detail
		
	quality:
		Number inputted via -d option
		Passed to loadpages() function in functions/btfunctions.h
		Determines Magick++ API's "Quality" measure as well as pixel density
		Initialized at 100 (default)
		If input value is less than 75, quality is set to 75
		(Lower values can cause trouble with Magick++)
		Recommended not to use values higher than 300 (could crash, uses up a lot of memory -- if your computer can handle it, then by all means, go for it)
		
	minsize:
		When printing in segments, -m specifies the minimum segment size (default at 4 pages)
		
			i.e., say you're printing a 203-page book in 40 pages per segment (with -s 40)
			
				with the default minsize (4),
					5 segments of 40 would leave only 3 pages remaining,
					so with a minsize of 4, you have 4 segments of 40, and 1 final segment of 43
					
					
			say you're printing a 207-page book in 40 pages per segment
			
				with the default minsize of 4,
					you have 5 segments of 40 + 1 final segment of 7
					
				changing the minsize to 8, for example, would tack those 7 pages on to the penultimate segment
			
			
			4 is a reasonable minsize for most practical domestic applications --
				if you're printing at home, with an ordinary household printer, and an ordinary household stapler,
					-s 40 -m 4 is perfectly reasonable
					
			however, for more advanced or industrial applications you may decide on:
					-s 160 -m 16, for example
	
	
	rescaling / outwidth/outheight / widthpresets / heightpresets:
		The first: A boolean activated if the user types -t
		The second (pair): The width/height in inches to transform to
		The third (pair): A set of presets (currently us-letter and a4)
		The first and second(pair) are passed to mayberescale() in functions/btfunctions.h
	************/
	int c;
	
	opterr = 0;
	
	while ((c = getopt(argc, argv, "ci:o:r:s:p:d:t:m:ghqfvblV")) != -1)
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
				cout << countpages(infilestr, false, false); // -p intentionally does not output a newline to stdout
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
				if (!iswritable(outfile)) {
					cerr << "Error: Output path is not writable" << endl;
					return 1;
				}
				break;
			case 'r':
				rangeflag = true;
				rangevalue = optarg;
				break;
			case 's':
				if (!is_number(optarg)) {
					cerr << "Error: Invalid (non-numeric) segment size '" << optarg << "'" << endl;
					return 1;
				}
				
				segsize = stoi(optarg);
				
				if (segsize < 4) {
					cerr << "Error: Segment size cannot be shorter than 4 pages" << endl;
					return 1;
				}
				break;
			case 'm':
				if (!is_number(optarg)) {
					cerr << "Error: Invalid (non-numeric) minsize '" << optarg << "'" << endl;
					return 1;
				}
				
				minsize = stoi(optarg);
				break;
			case 'd':
				if (!is_number(optarg)) {
					cerr << "Error: Invalid (non-numeric) pixel density '" << optarg << "'" << endl;
					return 1;
				}
				
				quality = stoi(optarg);
				
				if (quality < 75) {
					quality = 75;
				}
				break;
			case 't':
				if (!widthpresets[optarg]) {
				
					vector<string> customsize = explode(optarg, 'x');
		
					if (customsize.size() < 2 || !is_decimal(customsize[0]) || !is_decimal(customsize[1])) {
						cerr << "Error: Unrecognized transform '" << optarg << "'" << endl;
						cerr << "Valid options:" << endl;
					
						map<string, double>::iterator iter = heightpresets.begin();
						while (iter != heightpresets.end()) {
							cout << " " << iter->first << endl;
							iter++;
						}
						cerr << " or a custom size in inches, e.g: 8.5x11" << endl;
						return 1;
					}
					
					rescaling = true;
					outwidth = stod(customsize[0]);
					outheight = stod(customsize[1]);
					
				} else {
					rescaling = true;
					outwidth = widthpresets[optarg];
					outheight = heightpresets[optarg];
				}
				numstages = 3;
				break;
			case 'v':
				verbose = true;
				break;
			case 'b':
				bookthief = true;
				break;
			case 'l':
				landscapeflip = true;
				break;
			case 'h':
				cout << helpstring;
				return 0;
				break;
			case 'q':
				cout << infostring;
				return 0;
				break;
			case 'V':
				cout << versionstring;
				return 0;
				break;
			case '?':
				if (optopt == 'i' || optopt == 'o' || optopt == 'r' || optopt == 's' || optopt == 'p' || optopt == 'd' || optopt == 'm') {
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
		cerr << helpstring;
		return 1;
	}
	
	if (outfile == NULL) {
		cerr << helpstring;
		return 1;
	}
	
	string outstring = (string)outfile;
	
	if (!has_ending(outstring, ".pdf")) {
		outstring = outstring + ".pdf";
	}
	
	if (file_exists(outstring) && overwrite == false) {
		cerr << "Error: File '" << outstring << "' already exists!" << endl;
		return 1;
	}
	
	string infilestr(infile);
	int pagecount = countpages(infilestr, verbose, checkflag);
	
	if (rangeflag == true) {
	
		vector<string> multiranges = explode(rangevalue, ','); // Permitting multi-range printing (ie, -r 1-10,30-40,42-46) separated by commas
		
		
		
		for (int i=0;i<multiranges.size();i++) {
		
			vector<string> singlerange = explode(multiranges[i], '-');
			
			if (singlerange.size() > 2) {
				cerr << "Error: Invalid range '" << multiranges[i] << "'" << endl;
				return 1;
			}
			
			if (singlerange.size() > 1) {
			
				if (!is_number(singlerange[0]) || !is_number(singlerange[1])) {
					cerr << "Error: Invalid (non-numeric) range '" << multiranges[i] << "'" << endl;
					return 1;
				}
			
				int startpage = stoi(singlerange[0]);
				int endpage = stoi(singlerange[1]);
			
				if (startpage == endpage || startpage == 0) {
					cerr << "Error: Invalid range '" << multiranges[i] << "'" << endl;
					return 1;
				}
				
				
				if (max(startpage,endpage) > pagecount) {
					cerr << "Error: Given range value '" << multiranges[i] << "' out of range for supplied PDF" << endl;
					return 1;
				}
				
				
				for (int x=startpage-1;x>=endpage-1;x--) {
					finalpageselection.push_back(x); // Descending ranges (ie, -r 10-1)
				}
				
				for (int x=startpage-1;x<endpage;x++) {
					finalpageselection.push_back(x); // Ascending (normal) ranges (ie, -r 1-10)
				}
			
			} else if (singlerange.size() == 1) {
				if (!is_number(singlerange[0])) {
					cerr << "Error: Invalid (non-numeric) range '" << multiranges[i] << "'" << endl;
					return 1;
				}
				int singlepagerange = stoi(singlerange[0]);
				if (singlepagerange > pagecount) {
					cerr << "Error: Page '" << singlepagerange << "' out of range for supplied PDF" << endl;
					return 1;
				}
				finalpageselection.push_back(singlepagerange-1);
			} else {
				continue; // User inputted two commas by mistake (-r 1-20,21-40,,,,,,44-45)
			}
		
		}

	} else {
		for (int i=0;i<pagecount;i++) {
			finalpageselection.push_back(i);
		}
	}
	
	InitializeMagick(*argv);

	try {
		
		
		if (rangeflag == true) {
			
			pagecount = finalpageselection.size();
			
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
		
		if (finalsegsize < minsize && segcount > 1) {
			segcount = segcount - 1;
			finalsegsize = (pagecount % segsize) + segsize;
		}
		
		int thisseg = 1;
		
		int firstpage = 0;
		
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
			vector<Image> loaded = loadpages(segsize, infile, firstpage, finalpageselection, grayscale, lastpageblank, extrablanks, verbose, bookthief, segcount, thisseg, quality, numstages);		
			vector<Image> pamphlet = mayberescale(makepamphlet(loaded, verbose, bookthief, segcount, thisseg, numstages, landscapeflip, quality), rescaling, outwidth, outheight, quality, verbose, bookthief, segcount, thisseg, numstages);
			loaded.clear();
			if (verbose == true) {
				cout << endl << "Writing to file..." << endl;
			}
			writeImages(pamphlet.begin(), pamphlet.end(), outstring);
			
			
			pamphlet.clear(); // clear memory early for the sake of the user's machine
					// these kinds of operations can very easily take up gigabytes of memory
					// there's no sense holding on to all that memory until it's replaced, when we can free it up immediately

			double dpercentdone = (double)thisseg/segcount;
			int percentdone = floor(dpercentdone * 100);
			if (bookthief == true) {
				cout << percentdone << "%" << endl;
			}
		
			thisseg = 2;
			
			while (thisseg < segcount && thisseg > 1) {
				firstpage = segsize*(thisseg-1);
				
				string newname = outstring.substr(0, outstring.size()-4) + "-part" + to_string(thisseg) + ".pdf";
				
				loaded = loadpages(segsize, infile, firstpage, finalpageselection, grayscale, lastpageblank, extrablanks, verbose, bookthief, segcount, thisseg, quality, numstages);
				pamphlet = mayberescale(makepamphlet(loaded, verbose, bookthief, segcount, thisseg, numstages, landscapeflip, quality), rescaling, outwidth, outheight, quality, verbose, bookthief, segcount, thisseg, numstages);
				loaded.clear();
				if (verbose == true) {
					cout << endl << "Writing to file..." << endl;
				}
				writeImages(pamphlet.begin(), pamphlet.end(), newname);
				
				
				pamphlet.clear(); // clear memory early for the sake of the user's machine, see above
				
				
				dpercentdone = (double)thisseg/segcount;
				percentdone = floor(dpercentdone * 100);
				if (bookthief == true) {
					cout << percentdone << "%" << endl;
				}
				
				thisseg = thisseg + 1;
			}
			
			firstpage = segsize*(thisseg-1);
			
			string newname = outstring.substr(0, outstring.size()-4) + "-part" + to_string(thisseg) + ".pdf";

			loaded = loadpages(finalsegsize, infile, firstpage, finalpageselection, grayscale, flastpageblank, fextrablanks, verbose, bookthief, segcount, thisseg, quality, numstages);
			pamphlet = mayberescale(makepamphlet(loaded, verbose, bookthief, segcount, thisseg, numstages, landscapeflip, quality), rescaling, outwidth, outheight, quality, verbose, bookthief, segcount, thisseg, numstages);
			if (verbose == true) {
				cout << endl << "Writing to file..." << endl;
			}
			writeImages(pamphlet.begin(), pamphlet.end(), newname);
			
			if (bookthief == true) {
				cout << "100%" << endl;
			}
			cout << "Done!" << endl;
			return 0;
		}
		
		vector<Image> loaded = loadpages(finalsegsize, infile, firstpage, finalpageselection, grayscale, flastpageblank, fextrablanks, verbose, bookthief, segcount, thisseg, quality, numstages);
		vector<Image> pamphlet = mayberescale(makepamphlet(loaded, verbose, bookthief, segcount, thisseg, numstages, landscapeflip, quality), rescaling, outwidth, outheight, quality, verbose, bookthief, segcount, thisseg, numstages);
		if (verbose == true) {
			cout << endl << "Writing to file..." << endl;
		}
		writeImages(pamphlet.begin(), pamphlet.end(), outstring);
		
		if (bookthief == true) {
			cout << "100%" << endl;
		}
		cout << endl << "Done!" << endl;
		return 0;
		
	}
	catch( Exception &error_ )
	{
		cerr << "Error:\n" << error_.what() << endl;
		return 1;
	}
	return 0;
}
