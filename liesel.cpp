/***************************************************************
 * Name:      liesel
 * Version:   10.1
 * Author:    rail5 (andrew@rail5.org)
 * Created:   2022-01-27
 * Copyright: rail5 (https://rail5.org)
 * License:   GNU GPL V3
 **************************************************************/

#include "includes.h"

#define HAS_OPTIONAL_ARGUMENT \
    ((optarg == NULL && optind < argc && argv[optind][0] != '-') \
     ? (bool) (optarg = argv[optind++]) \
     : (optarg != NULL))


using namespace std;

int main(int argc,char **argv)
{

	const string versionstring = "10.0";

	const string helpstring = "Liesel " + versionstring + "\n\nUsage:\nliesel -i input-file.pdf -o output-file.pdf\n\nOptions:\n\n  -i\n  --input\n    PDF to convert\n\n  -o\n  --output\n    New file for converted PDF\n\n  -I\n  --stdin\n    Read PDF from STDIN rather than file\n    e.g: liesel -I -o output.pdf < input.pdf\n\n  -O\n  --stdout\n    Write output PDF to STDOUT rather than file\n    e.g: liesel -i input.pdf -O > output.pdf\n\n  -e\n  --export\n    Export example/preview JPEG image of selected pages\n    e.g: -i infile.pdf -e 5,6 -o preview.jpeg\n    e.g: --input infile.pdf --export 5,6 --output preview.jpeg\n\n  -g\n  --greyscale\n  --grayscale\n    Convert PDF to greyscale/black and white\n\n  -r\n  --range\n    Print only specified range of pages (in the order supplied)\n    e.g: -r 1-12\n    e.g: --range 15-20,25-30\n    e.g: -r 10,9,5,2,1\n    e.g: --range 20-10\n\n  -s\n  --segment\n    Print output PDFs in segments of a given size\n    e.g: -s 40\n      (produces multiple PDFs of 40 pages each)\n\n  -m\n  --minsize\n    Specify minimum segment size (default is 4)\n    e.g: -m 8\n\n  -f\n  --force\n    Force overwrites\n      (do not warn about files already existing)\n\n  -v\n  --verbose\n    Verbose mode\n\n  -b\n  --bookthief\n  --progress\n    Show progress (percentage done)\n\n  -d\n  --density\n  --quality\n  --ppi\n    Specify pixels-per-inch density/quality (default is 100)\n    e.g: -d 200\n\n  -t\n  --transform\n  --rescale\n    Transform output PDF to print on a specific size paper\n      e.g: -t 8.5x11\n\n  -l\n  --landscape\n  --duplex\n    Duplex printer \"landscape\" flipping compatibility\n      (flips every other page)\n\n  -k\n  --threshold\n    Convert to pure black-and-white (not grayscale) with given threshold value between 0-100\n    e.g: -k 70\n      will convert any color with brightness under 70% to pure black, and any brighter color to pure white\n      this option is particularly useful in printing PDFs of scanned books with yellowed pages etc\n\n  -C\n  --crop\n    Crop pages according to specified values\n    e.g: -C 10,20,30,40\n      order: left,right,top,bottom\n      cuts 10% from the left side, 20% from the right side, etc\n\n  -w\n  --widen\n    Widen center margins according to specified value\n    e.g: -w 30\n      (adds blank space between pages)\n\n  -a\n  --auto-widen\n    Auto-widen center margins\n    e.g: -a (completely automatic)\n    e.g: -a 30 (auto, with a maximum of 30)\n      (progressively widens center margins towards the middle of the booklet)\n      (if -w / --widen is also specified, -w / --widen acts as the minimum margin)\n\n  -D\n  --divide\n    Divide each page into two\n      divides the left half and the right half into separate pages\n      this option is particularly useful in printing PDFs of scanned books which haven't already separated the pages\n\n  -p\n  --pages\n    Count pages of input PDF and exit\n\n  -c\n  --check\n    Check validity of command, and do not execute\n\n  -B\n  --output-settings\n    Prints the parameters/settings you've provided in XML format\n\n  -h\n  --help\n    Print this help message\n\n  -q\n  --info\n    Print program info\n\n  -V\n  --version\n    Print version number\n\nExample:\n  liesel -i some-book.pdf -g -r 64-77 -f -d 150 -v -b -o ready-to-print.pdf\n  liesel -i some-book.pdf -r 100-300,350-400,1-10 -s 40 -t 8.25x11.75 -m 16 -o ready-to-print.pdf\n  liesel --input some-book.pdf --range 1,5,7,3,1,50 --landscape --output ready-to-print.pdf\n  liesel -p some-book.pdf\n  liesel -c -i some-book.pdf -o output.pdf\n";
	
	const string infostring = "BookThief + Liesel Copyright (C) 2022 rail5\n" + versionstring + "\n\nThis program comes with ABSOLUTELY NO WARRANTY.\nThis is free software (GNU GPL V3), and you are welcome to redistribute it under certain conditions.\n\n0. Liesel takes an ordinary PDF and converts it into a booklet-ready PDF to be home-printed\n1. BookThief is a GUI front-end which merely makes calls to Liesel\n2. The source code for both programs is freely available online\n3. Liesel depends on GraphicsMagick and Poppler, two free (GPL V3-compatible) libraries\n";

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
	bool alterthreshold = false;
	bool cropflag = false;
	bool widenflag = false;
	bool exportflag = false;
	bool dividepages = false;
	bool segflag = false;
	bool automargin = false;
	
	bool optout = false;
	bool noinfile = false;
	bool nooutfile = false;

	bool receivedinfile = false;
	bool pdfstdin = false;
	bool pdfstdout = false;
	string binaryinput = "";
	
	char *infile = NULL;
	char *outfile = NULL;
	char *rangevalue = NULL;
	char *exportvalue = NULL;
	
	string kparam = "";
	
	string crop = "0,0,0,0";
	vector<int> cropvalues = {0, 0, 0, 0};
	
	int widenby = 0;
	
	int maxmargin = 0;

	int segsize = 0;
	
	vector<int> finalpageselection;
	
	int numstages = 2;
	
	int quality = 100;
	
	int minsize = 4;
	
	double threshold = 32767.5;

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
	
	int option_index = 0;
	
	static struct option long_options[] =
	{
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{"export", required_argument, 0, 'e'},
		{"range", required_argument, 0, 'r'},
		{"segment", required_argument, 0, 's'},
		{"minsize", required_argument, 0, 'm'},
		{"density", required_argument, 0, 'd'},
		{"quality", required_argument, 0, 'd'},
		{"ppi", required_argument, 0, 'd'},
		{"transform", required_argument, 0, 't'},
		{"rescale", required_argument, 0, 't'},
		{"threshold", required_argument, 0, 'k'},
		{"crop", required_argument, 0, 'C'},
		{"widen", required_argument, 0, 'w'},
		{"pages", required_argument, 0, 'p'},
		
		{"stdin", no_argument, 0, 'I'},
		{"stdout", no_argument, 0, 'O'},
		{"grayscale", no_argument, 0, 'g'},
		{"greyscale", no_argument, 0, 'g'},
		{"force", no_argument, 0, 'f'},
		{"verbose", no_argument, 0, 'v'},
		{"progress", no_argument, 0, 'b'},
		{"bookthief", no_argument, 0, 'b'},
		{"landscape", no_argument, 0, 'l'},
		{"duplex", no_argument, 0, 'l'},
		{"auto-widen", no_argument, 0, 'a'},
		{"divide", no_argument, 0, 'D'},
		{"check", no_argument, 0, 'c'},
		{"help", no_argument, 0, 'h'},
		{"info", no_argument, 0, 'q'},
		{"version", no_argument, 0, 'V'},
		{"output-settings", no_argument, 0, 'B'},
		{0, 0, 0, 0}
	};
	
	while ((c = getopt_long(argc, argv, "a::cIi:Oo:r:s:p:d:t:m:e:k:C:w:ghqfvblDVB", long_options, &option_index)) != -1)
		switch(c) {
			case 'B':
				optout = true;
				verbose = false;
				break;
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
				
				if (!checkin(infile, true)) {
					return 1;
				}
				
				InitializeMagick(*argv);
	
				string infilestr(infile);
				
				if (checkflag == true) {
					cout << "OK";
					return 0;
				}
				cout << countpages(infilestr, false, false, false); // -p intentionally does not output a newline to stdout
				return 0;
				
				break;
			}
			case 'i':
				infile = optarg;
				if (!checkin(infile, !optout) && !optout) {
					return 1;
				} else if (!checkin(infile, !optout) && optout) {
					noinfile = true;
				}
				receivedinfile = true;
				break;
			case 'I':
				pdfstdin = true;
				break;
			case 'o':
				outfile = optarg;
				if (!iswritable(outfile) && !optout) {
					cerr << "Error: Output path " << outfile << " is not writable" << endl;
					return 1;
				}
				break;
			case 'O':
				pdfstdout = true;
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
				segflag = true;
				
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
				if (!optout) {
					verbose = true;
				}
				break;
			case 'b':
				bookthief = true;
				break;
			case 'l':
				landscapeflip = true;
				break;
			case 'k':
				alterthreshold = true;
				if (!is_number(optarg)) {
					cerr << "Error: Invalid (non-numeric) threshold supplied '" << optarg << "'" << endl;
					return 1;
				}
				kparam = optarg;
				#ifdef _WIN32
				threshold = stod(optarg) * 2.55;
				// MaxRGB on Windows is 255 (therefore -k 100 should = -k 255)
				#else
				// MaxRGB on *Nix is 65535 (etc)
				threshold = stod(optarg) * 655.35;
				#endif
				break;
			case 'C':
				cropflag = true;
				crop = (string)optarg;
				break;
			case 'w':
				widenflag = true;
				
				if (!is_number(optarg)) {
					cerr << "Error: Invalid (non-numeric) margin '" << optarg << "'" << endl;
					return 1;
				}
				
				widenby = stoi(optarg);
				
				if (widenby > 100) {
					widenby = 100;
				}
				break;
			case 'D':
				dividepages = true;
				break;
			case 'e':
				exportflag = true;
				exportvalue = optarg;
				break;
			case 'a':
				automargin = true;
				if (HAS_OPTIONAL_ARGUMENT) {
					if (is_number(optarg)) {
						maxmargin = stoi(optarg);
					} else {
						cerr << "Error: Invalid (non-numeric) maximum margin '" << optarg << "'" << endl;
						return 1;
					}
				}
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
				if (optopt == 'i' || optopt == 'o' || optopt == 'r' || optopt == 's' || optopt == 'p' || optopt == 'd' || optopt == 'm' || optopt == 'e' || optopt == 'k' || optopt == 'C' || optopt == 'w') {
					fprintf(stderr, "Option -%c requires an argument\nUse liesel -h for help\n", optopt);
				} else if (isprint(optopt)) {
					fprintf(stderr, "Unknown option `-%c'\nUse liesel -h for help\n", optopt);
				} else {
					fprintf(stderr, "Unknown option character `\\x%x'\nUse liesel -h for help\n", optopt);
				}
			return 1;
			default:
				abort();
			}
	
	if (!receivedinfile && pdfstdin) {
		// Read PDF contents from STDIN if:
		// -i was not specified (!receivedinfile), AND
		// -I WAS specified (pdfstdin)
		const size_t STDIN_BUFFER_SIZE = 1024;
		
		try {
			freopen(nullptr, "rb", stdin);
			
			if (ferror(stdin)) {
				throw runtime_error(strerror(errno));
			}
			
			size_t len;
			array<char, STDIN_BUFFER_SIZE> buf;
			
			vector<char> stdinput;
			
			while ((len = fread(buf.data(), sizeof(buf[0]), buf.size(), stdin)) > 0) {
				if (ferror(stdin) && !feof(stdin)) {
					throw runtime_error(strerror(errno));
				}
				
				stdinput.insert(stdinput.end(), buf.data(), buf.data() + len);
			}
		
			
		
			for (long unsigned int i=0;i<stdinput.size();i++) {
				binaryinput = binaryinput + stdinput[i];
			}
			
		} catch (exception const& e) {
			cerr << e.what() << endl;
			return 1;
		}
	}
			
	if (infile == NULL && pdfstdin == false) {
		if (!optout) {
			cerr << helpstring;
			return 1;
		}
		noinfile = true; // Only set if -B was specified, ie infile not needed
	}
	
	if (infile != NULL && pdfstdin == true) {
		// Read from provided file by preference rather than STDIN if both -i and -I are supplied
		pdfstdin = false;
	}
	
	if (outfile == NULL && pdfstdout == false) {
		if (!optout) {
			cerr << helpstring;
			return 1;
		}
		nooutfile = true;
	}
	
	if (outfile != NULL && pdfstdout == true) {
		pdfstdout = false;
	}
	
	if (pdfstdout) {
		outfile = (char*)"0"; // needs to be initialized
		
		verbose = false;
		bookthief = false;
		
		if (segflag) {
			cerr << "Warning: Printing multiple, separate PDFs to STDOUT" << endl << endl;
		}
		
	}
	
	if (pdfstdin) {
		infile = (char*)"0"; // also needs to be initialized
	}
	
	string outstring;
	
	if (!optout) {
		outstring = (string)outfile;
	
		if (!has_ending(outstring, ".pdf") && exportflag == false) {
			outstring = outstring + ".pdf";
		}
	
		if (!has_ending(outstring, ".jpeg") && exportflag == true) {
			outstring = outstring + ".jpeg";
		}
	
		if (file_exists(outstring) && overwrite == false) {
			cerr << "Error: File '" << outstring << "' already exists!" << endl;
			return 1;
		}
	}
		
	if (dividepages == true && segflag == true) {
		segsize = segsize / 2;
		if (segsize < 4) {
			segsize = 4;
		}
	}
	
	if (cropflag == true) {
		vector<string> tmpcropvector = explode(crop, ',');
		if (tmpcropvector.size() != 4) {
			cerr << "Error: Invalid crop '" << crop << "'" << endl;
			return 1;
		}
		
		if (!is_number(tmpcropvector[0]) || !is_number(tmpcropvector[1]) || !is_number(tmpcropvector[2]) || !is_number(tmpcropvector[3])) {
			cerr << "Error: Invalid (non-numeric) crop '" << crop << "'" << endl;
			return 1;
		}
		
		cropvalues[0] = stoi(tmpcropvector[0]);
		cropvalues[1] = stoi(tmpcropvector[1]);
		cropvalues[2] = stoi(tmpcropvector[2]);
		cropvalues[3] = stoi(tmpcropvector[3]);
	
	}
	
	if (optout) {
		string xmloutput = "<settings>\n";
		if (!noinfile) {
			xmloutput = xmloutput + "<infile>" + infile + "</infile>\n";
		}
		if (!nooutfile) {
			xmloutput = xmloutput + "<outfile>" + outfile + "</outfile>\n";
		}
		if (automargin) {
			xmloutput = xmloutput + "<automargin>" + to_string(maxmargin) + "</automargin>\n";
		}
		if (widenflag) {
			xmloutput = xmloutput + "<widen>" + to_string(widenby) + "</widen>\n";
		}
		if (bookthief) {
			xmloutput = xmloutput + "<bookthief>on</bookthief>\n";
		}
		if (crop != "0,0,0,0") {
			xmloutput = xmloutput + "<crop>" + crop + "</crop>\n";
		}
		if (dividepages) {
			xmloutput = xmloutput + "<divide>on</divide>\n";
		}
		if (overwrite) {
			xmloutput = xmloutput + "<overwrite>on</overwrite>\n";
		}
		if (grayscale) {
			xmloutput = xmloutput + "<grayscale>on</grayscale>\n";
		}
		if (alterthreshold) {
			xmloutput = xmloutput + "<threshold>" + kparam + "</threshold>\n";
		}
		if (landscapeflip) {
			xmloutput = xmloutput + "<longedge>on</longedge>\n";
		}
		if (rangeflag) {
			xmloutput = xmloutput + "<range>" + rangevalue + "</range>\n";
		}
		if (segflag) {
			xmloutput = xmloutput + "<segment>" + to_string(segsize) + "</segment>\n";
		}
		if (rescaling) {
			string stroutwidth = to_string(outwidth);
			string stroutheight = to_string(outheight);
			while (has_ending(stroutwidth, "0") && !has_ending(stroutwidth, ".0")) {
				stroutwidth.pop_back();
			}
			while (has_ending(stroutheight, "0") && !has_ending(stroutheight, ".0")) {
				stroutheight.pop_back();
			}
			
			xmloutput = xmloutput + "<transform>" + stroutwidth + "x" + stroutheight + "</transform>\n";
		}
		xmloutput = xmloutput + "<minsize>" + to_string(minsize) + "</minsize>\n";
		xmloutput = xmloutput + "<quality>" + to_string(quality) + "</quality>\n";
		
		xmloutput = xmloutput + "</settings>";
		
		cout << xmloutput << endl;
		return 0;
		
	}
	
	string infilestr;
	if (!pdfstdin) {
		infilestr = infile;
	} else {
		infilestr = binaryinput;
	}
	int pagecount = countpages(infilestr, verbose, checkflag, pdfstdin);
	
	if (rangeflag == true) {
	
		vector<string> multiranges = explode(rangevalue, ','); // Permitting multi-range printing (ie, -r 1-10,30-40,42-46) separated by commas
		
		
		
		for (long unsigned int i=0;i<multiranges.size();i++) {
		
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
			
				if (startpage == endpage || startpage == 0 || endpage == 0) {
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
				
				if (singlepagerange == 0) {
					cerr << "Error: Invalid page '0'" << endl;
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
	
	if (exportflag == true) {
		vector<string> toexport = explode(exportvalue, ',');
		
		if (toexport.size() > 2) {
			cerr << "Error: Can only export a maximum of 2 pages" << endl;
			return 1;
		}
		
		if (toexport.size() > 1) {
		
			if (!is_number(toexport[0]) || !is_number(toexport[1])) {
				cerr << "Error: Invalid (non-numeric) export page numbers '" << exportvalue << "'" << endl;
				return 1;
			}
			int exportpageone = stoi(toexport[0]);
			int exportpagetwo = stoi(toexport[1]);
			
			if (max(exportpageone, exportpagetwo) > pagecount) {
				cerr << "Error: Given 'export' pages  '" << exportvalue << "' out of range for supplied PDF" << endl;
				return 1;
			}
			
			if (exportpageone == 0 || exportpagetwo == 0) {
				cerr << "Error: Invalid export page numbers '" << exportvalue << "'" << endl;
				return 1;
			}
			
			finalpageselection.clear();
			finalpageselection.push_back(exportpagetwo-1);
			finalpageselection.push_back(exportpageone-1);
			
		} else {
			cerr << "Error: You must select 2 pages for export" << endl;
			return 1;
		}
	}
	
	InitializeMagick(*argv);

	try {
		if (rangeflag == true || exportflag == true) {
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
			if (checksegout(outstring, segcount, overwrite, pdfstdout) != true) {
				return 1;
			}
		}
		
		if (checkflag == true) {
			cout << "OK";
			return 0;
		}
		
		int revsegsize = segsize;
		int revfinalsegsize = finalsegsize + (finalsegsize * dividepages);
		
		if (segsize % 2 != 0) {
			lastpageblank = true;
			revsegsize = segsize + 1;
		}
		
		if (revfinalsegsize % 2 != 0 && exportflag == false) {
			flastpageblank = true;
			revfinalsegsize = revfinalsegsize + 1;
		}
		
		if (revsegsize % 4 != 0) {
			extrablanks = true;
		}
		
		if (revfinalsegsize % 4 != 0 && exportflag == false) {
			fextrablanks = true;
		}

		
		
		
		while (thisseg < segcount) {
			
			firstpage = segsize*(thisseg-1);
				
			string newname = outstring.substr(0, outstring.size()-4) + "-part" + to_string(thisseg) + ".pdf";
				
			vector<Image> loaded = loadpages(segsize, infilestr, pdfstdin, firstpage, finalpageselection, grayscale, alterthreshold, threshold, cropflag, cropvalues, dividepages, lastpageblank, extrablanks, verbose, bookthief, segcount, thisseg, quality, numstages);
			vector<Image> pamphlet = mayberescale(makepamphlet(loaded, verbose, bookthief, segcount, thisseg, numstages, landscapeflip, quality, widenflag, widenby, exportflag, dividepages, automargin, maxmargin), rescaling, outwidth, outheight, quality, verbose, bookthief, segcount, thisseg, numstages, exportflag);
			loaded.clear();
			if (verbose == true) {
				cout << endl << "Writing to file..." << endl;
			}
			if (!pdfstdout) {
				writeImages(pamphlet.begin(), pamphlet.end(), newname);
			} else {
				Blob inmemory; // writeImages() will write to this Blob
			
				for (long unsigned int i=0;i<pamphlet.size();i++) {
					pamphlet[i].magick("pdf"); // Explicitly set output type
				}
			
				writeImages(pamphlet.begin(), pamphlet.end(), &inmemory, true); // Write to Blob
			
				unsigned char* outputpdfyo = (unsigned char*)inmemory.data(); // Access the bytes of the Blob
		
				for (long unsigned int i=0;i<inmemory.length();i++) {
					printf("%c", outputpdfyo[i]);
				}
		
			}
				
				
			pamphlet.clear(); // clear memory early for the sake of the user's machine, see above
				
				
			double dpercentdone = (double)thisseg/segcount;
			int percentdone = floor(dpercentdone * 100);
			if (bookthief == true && pdfstdout == false) {
				cout << percentdone << "%" << endl;
			}
				
			thisseg = thisseg + 1;
		}
			
		firstpage = segsize*(thisseg-1);
		
		string appendtofname = ".pdf";
		
		if (segcount > 1) {
			appendtofname = "-part" + to_string(thisseg) + ".pdf";
		}
		
		string newname = outstring;

		if (!exportflag) {
			newname = outstring.substr(0, outstring.size()-4) + appendtofname;
		}

		vector<Image> loaded = loadpages(finalsegsize, infilestr, pdfstdin, firstpage, finalpageselection, grayscale, alterthreshold, threshold, cropflag, cropvalues, dividepages, flastpageblank, fextrablanks, verbose, bookthief, segcount, thisseg, quality, numstages);
		vector<Image> pamphlet = mayberescale(makepamphlet(loaded, verbose, bookthief, segcount, thisseg, numstages, landscapeflip, quality, widenflag, widenby, exportflag, dividepages, automargin, maxmargin), rescaling, outwidth, outheight, quality, verbose, bookthief, segcount, thisseg, numstages, exportflag);
		
		if (verbose == true && pdfstdout == false) {
			cout << endl << "Writing to file..." << endl;
		}
		
		if (!pdfstdout) {
			writeImages(pamphlet.begin(), pamphlet.end(), newname);
		} else {
			Blob inmemory; // writeImages() will write to this Blob
			
			for (long unsigned int i=0;i<pamphlet.size();i++) {
				pamphlet[i].magick("pdf"); // Explicitly set output type
			}
			
			writeImages(pamphlet.begin(), pamphlet.end(), &inmemory, true); // Write to Blob
			
			unsigned char* outputpdfyo = (unsigned char*)inmemory.data(); // Access the bytes of the Blob
		
			for (long unsigned int i=0;i<inmemory.length();i++) {
				printf("%c", outputpdfyo[i]);
			}
		
		}
		
		if (bookthief == true && pdfstdout == false) {
			cout << "100%" << endl;
		}
		if (!pdfstdout) {
			cout << "Done!" << endl;
		}
		return 0;
		
	}
	catch( Exception &error_ )
	{
		cerr << "Error:\n" << error_.what() << endl;
		return 1;
	}
	return 0;
}
