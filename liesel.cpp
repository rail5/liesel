/***************************************************************
 * Name:      liesel
 * Version:   3.0
 * Author:    rail5 (andrew@rail5.org)
 * Created:   2021-08-17
 * Copyright: rail5 (https://rail5.org)
 * License:   GNU GPL v3.0
 **************************************************************/

#ifndef IOSTREAM
#define IOSTREAM
#include <iostream>
#endif

#include "functions/itoa.h"
#include "functions/explode.h"
#include "functions/has_ending.h"
#include "functions/is_number.h"
#include "functions/file_exists.h"
#include "functions/btfunctions.h"

using namespace std;

int main(int argc,char **argv)
{

	const char* helpstring = "Usage:\nliesel -i input-file.pdf -o output-file.pdf\n\nOptions:\n\n  -i\n    PDF to convert\n\n  -o\n    New file for converted PDF\n\n  -g\n    Convert PDF to greyscale/black and white\n\n  -r\n    Print only within specified range\n    e.g: -r 1-12\n\n  -s\n    Print output PDFs in segments of a given size\n    e.g: -s 40\n      (produces multiple PDFs)\n\n  -p\n    Count pages of input PDF and exit\n\n  -c\n    Check validity of command, and do not execute\n\n  -h\n    Print this help message\n\nExample:\n  liesel -i some-book.pdf -g -r 64-77 -o ready-to-print.pdf\n  liesel -i some-book.pdf -r 100-300 -s 40 -o ready-to-print.pdf\n  liesel -p some-book.pdf\n  liesel -c -i some-book.pdf -o output.pdf\n";

	bool grayscale = false;
	bool rangeflag = false;
	bool checkflag = false;
	
	char *infile = NULL;
	char *outfile = NULL;
	char *rangevalue = NULL;

	int segsize = 0;	
	int rangestart = 0;
	int rangeend;
	int rangelength;
	
	
	/************
	grayscale:
		A flag tripped when the user types -g
		If this flag is tripped to return "true," Liesel converts an inputted color PDF to black&white
	
	rangeflag:
		A flag tripped when the user specifies a range with -r
		If this flag is tripped to return "true," Liesel first checks the validity of the range, and then modifies the call to loadpages() with the range-specific info
		
	checkflag:
		A flag tripped when the user types -c
		If this flag is tripped to return "true," Liesel checks whether the command is valid, and halts before execution (does NOT execute the command issued). Liesel returns either an error, or "OK"
	
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
	
	while ((c = getopt(argc, argv, "ci:o:r:s:p:gh")) != -1)
		switch(c) {
			case 'c':
				checkflag = true;
				break;
			case 'g':
				grayscale = true;
				break;
			case 'p':
			{
				infile = optarg;
				
				InitializeMagick(*argv);
	
				std::string infilestr(infile);

				char counted[33];
				itoa(countpages(infilestr), counted, 10);
				if (checkflag == true) {
					std::cout << "OK";
					return 0;
				}
				std::cout << counted;
				return 0;
				
				break;
			}
			case 'i':
				infile = optarg;
				
				if (!file_exists(infile)) {
					std::cout << "Error: File '" << infile << "' not found" << endl;
					return 1;
				}
	
				if (!has_ending((std::string)infile, ".pdf")) {
					std::cout << "Error: At this stage, Liesel only supports PDFs\nPlease look for a future version to support other formats" << endl;
					return 1;
				}
				
				break;
			case 'o':
				outfile = optarg;
				
				if (file_exists(outfile)) {
					std::cout << "Error: File '" << outfile << "' already exists!" << endl;
					return 1;
				}
	
				if (!has_ending((std::string)outfile, ".pdf")) {
					strcat(outfile, ".pdf");
				}
				
				break;
			case 'r':
				rangeflag = true;
				rangevalue = optarg;
				break;
			case 's':
				if (!is_number(optarg)) {
					std::cout << "Error: Invalid (non-numeric) segment size '" << optarg << "'" << endl;
					return 1;
				}
				
				segsize = std::stoi(optarg);
				
				if (segsize < 4) {
					std::cout << "Error: Segment size cannot be shorter than 4 pages" << endl;
					return 1;
				}
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
	
	if (outfile == NULL) {
		std::cout << helpstring;
		return 1;
	}
	
	if (rangeflag == true) {
		std::vector<std::string> rng = explode(rangevalue, '-');
		
		if (rng.size() < 2) {
			std::cout << "Error: Invalid range '" << rangevalue << "'" << endl;
			return 1;
		}
		
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
		
		if (rangeflag == true) {
			if (rangeend > pagecount) {
				std::cout << "Error: Given 'range' value out of range for supplied PDF" << endl;
				return 1;
			}
			
			pagecount = rangelength + 1;
			
		}
		
		if (checkflag == true) {
			std::cout << "OK";
			return 0;
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
		
		std::string outstring = (std::string)outfile;
		
		if (segcount > 1) {
			vector<Image> loaded = loadpages(segsize, infile, firstpage, grayscale);		
			std::list<Image> pamphlet = makepamphlet(segsize, loaded);
			writeImages(pamphlet.begin(), pamphlet.end(), outfile);
			
			double dpercentdone = (double)thisseg/segcount;
			int percentdone = floor(dpercentdone * 100);
			
			char percentchar[33];
			itoa(percentdone, percentchar, 10);
			
			std::cout << percentchar << "%" << endl;
		
			thisseg = 2;
			
			while (thisseg < segcount && thisseg > 1) {
				firstpage = rangestart + (segsize*(thisseg-1));
				
				char newname[256];
				strcpy(newname, outstring.substr(0, outstring.size()-4).c_str()); // remove .pdf extension
				
				char thiscounter[33];
				itoa(thisseg, thiscounter, 10);
				
				strcat(newname, "-part");
				strcat(newname, thiscounter);
				strcat(newname, ".pdf"); //ie, ourfile-part2.pdf
				
				loaded = loadpages(segsize, infile, firstpage, grayscale);
				pamphlet = makepamphlet(segsize, loaded);
				writeImages(pamphlet.begin(), pamphlet.end(), newname);
				
				dpercentdone = (double)thisseg/segcount;
				percentdone = floor(dpercentdone * 100);
				itoa(percentdone, percentchar, 10);
				std::cout << percentchar << "%" << endl;
				
				thisseg = thisseg + 1;
			}
			
			firstpage = rangestart + (segsize*(thisseg-1));
			
			char newname[256];
			strcpy(newname, outstring.substr(0, outstring.size()-4).c_str());
			char thiscounter[33];
			itoa(thisseg, thiscounter, 10);
			strcat(newname, "-part");
			strcat(newname, thiscounter);
			strcat(newname, ".pdf");
			
			loaded = loadpages(finalsegsize, infile, firstpage, grayscale);
			pamphlet = makepamphlet(finalsegsize, loaded);
			writeImages(pamphlet.begin(), pamphlet.end(), newname);
			
			std::cout << "100%" << endl;
			
			std::cout << "Done!" << endl;
			return 0;
		}
		
		vector<Image> loaded = loadpages(finalsegsize, infile, firstpage, grayscale);
		std::list<Image> pamphlet = makepamphlet(finalsegsize, loaded);
		writeImages(pamphlet.begin(), pamphlet.end(), outfile);
		
		std::cout << endl << "Done!" << endl;
		return 0;
		
	}
	catch( Exception &error_ )
	{
		std::cout << "Error:\n" << error_.what() << endl;
		return 1;
	}
	return 0;
}
