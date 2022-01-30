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

	const string versionstring = "11.0";

	const string helpstring = "Liesel " + versionstring + "\n\nUsage:\nliesel -i input-file.pdf -o output-file.pdf\n\nOptions:\n\n  -i\n  --input\n    PDF to convert\n\n  -o\n  --output\n    New file for converted PDF\n\n  -I\n  --stdin\n    Read PDF from STDIN rather than file\n    e.g: liesel -I -o output.pdf < input.pdf\n\n  -O\n  --stdout\n    Write output PDF to STDOUT rather than file\n    e.g: liesel -i input.pdf -O > output.pdf\n\n  -e\n  --export\n    Export example/preview JPEG image of selected pages\n    e.g: -i infile.pdf -e 5,6 -o preview.jpeg\n    e.g: --input infile.pdf --export 5,6 --output preview.jpeg\n\n  -g\n  --greyscale\n  --grayscale\n    Convert PDF to greyscale/black and white\n\n  -r\n  --range\n    Print only specified range of pages (in the order supplied)\n    e.g: -r 1-12\n    e.g: --range 15-20,25-30\n    e.g: -r 10,9,5,2,1\n    e.g: --range 20-10\n\n  -s\n  --segment\n    Print output PDFs in segments of a given size\n    e.g: -s 40\n      (produces multiple PDFs of 40 pages each)\n\n  -m\n  --minsize\n    Specify minimum segment size (default is 4)\n    e.g: -m 8\n\n  -f\n  --force\n    Force overwrites\n      (do not warn about files already existing)\n\n  -v\n  --verbose\n    Verbose mode\n\n  -b\n  --bookthief\n  --progress\n    Show progress (percentage done)\n\n  -d\n  --density\n  --quality\n  --ppi\n    Specify pixels-per-inch density/quality (default is 100)\n    e.g: -d 200\n\n  -t\n  --transform\n  --rescale\n    Transform output PDF to print on a specific size paper\n      e.g: -t 8.5x11\n\n  -l\n  --landscape\n  --duplex\n    Duplex printer \"landscape\" flipping compatibility\n      (flips every other page)\n\n  -k\n  --threshold\n    Convert to pure black-and-white (not grayscale) with given threshold value between 0-100\n    e.g: -k 70\n      will convert any color with brightness under 70% to pure black, and any brighter color to pure white\n      this option is particularly useful in printing PDFs of scanned books with yellowed pages etc\n\n  -C\n  --crop\n    Crop pages according to specified values\n    e.g: -C 10,20,30,40\n      order: left,right,top,bottom\n      cuts 10% from the left side, 20% from the right side, etc\n\n  -w\n  --widen\n    Widen center margins according to specified value\n    e.g: -w 30\n      (adds blank space between pages)\n\n  -a\n  --auto-widen\n    Auto-widen center margins\n    e.g: -a (completely automatic)\n    e.g: -a 30 (auto, with a maximum of 30)\n      (progressively widens center margins towards the middle of the booklet)\n      (if -w / --widen is also specified, -w / --widen acts as the minimum margin)\n\n  -D\n  --divide\n    Divide each page into two\n      divides the left half and the right half into separate pages\n      this option is particularly useful in printing PDFs of scanned books which haven't already separated the pages\n\n  -N\n  --no-booklet\n  --linear\n    Applies the changes requested (crop, color threshold, etc) and outputs linearly, without making a booklet\n      all features can be used except for center-margin widening\n\n  -p\n  --pages\n    Count pages of input PDF and exit\n\n  -c\n  --check\n    Check validity of command, and do not execute\n\n  -B\n  --output-settings\n    Prints the parameters/settings you've provided in XML format\n\n  -h\n  --help\n    Print this help message\n\n  -q\n  --info\n    Print program info\n\n  -V\n  --version\n    Print version number\n\nExample:\n  liesel -i some-book.pdf -g -r 64-77 -f -d 150 -v -b -o ready-to-print.pdf\n  liesel -i some-book.pdf -r 100-300,350-400,1-10 -s 40 -t 8.25x11.75 -m 16 -o ready-to-print.pdf\n  liesel --input some-book.pdf --range 1,5,7,3,1,50 --landscape --output ready-to-print.pdf\n  liesel -p some-book.pdf\n  liesel -c -i some-book.pdf -o output.pdf\n";
	
	const string infostring = "BookThief + Liesel Copyright (C) 2022 rail5\n" + versionstring + "\n\nThis program comes with ABSOLUTELY NO WARRANTY.\nThis is free software (GNU GPL V3), and you are welcome to redistribute it under certain conditions.\n\n0. Liesel takes an ordinary PDF and converts it into a booklet-ready PDF to be home-printed\n1. BookThief is a GUI front-end which merely makes calls to Liesel\n2. The source code for both programs is freely available online\n3. Liesel depends on GraphicsMagick and Poppler, two free (GPL V3-compatible) libraries\n";

	Liesel::Book thebook;
	
	bool rangeflag = false;
	
	bool overwrite = false;
	bool checkflag = false;
	bool verbose = false;
	bool bookthief = false;
	bool lastpageblank = false;
	bool flastpageblank = false;
	bool extrablanks = false;
	bool fextrablanks = false;
	
	bool exportflag = false;
	
	bool segflag = false;
	
	bool optout = false;
	bool noinfile = false;
	bool nooutfile = false;

	bool receivedinfile = false;
	bool pdfstdin = false;
	bool pdfstdout = false;
	
	string binaryinput = "";
	
	string infile = "";
	string outfile = "";
	string rangevalue = "";
	string exportvalue = "";
	
	string kparam = "";
	
	string crop = "0,0,0,0";

	int segsize = 0;
	
	int minsize = 4;
	
	map<string, double> widthpresets;
	map<string, double> heightpresets;
	
	widthpresets["us-letter"] = 8.5;
	heightpresets["us-letter"] = 11;
	
	widthpresets["a4"] = 8.3;
	heightpresets["a4"] = 11.7;

	
	/************
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
		{"no-booklet", no_argument, 0, 'N'},
		{"linear", no_argument, 0, 'N'},
		{"check", no_argument, 0, 'c'},
		{"help", no_argument, 0, 'h'},
		{"info", no_argument, 0, 'q'},
		{"version", no_argument, 0, 'V'},
		{"output-settings", no_argument, 0, 'B'},
		{0, 0, 0, 0}
	};
	
	while ((c = getopt_long(argc, argv, "a::cIi:Oo:r:s:p:d:t:m:e:k:C:w:ghqfvblDNVB", long_options, &option_index)) != -1)
		switch(c) {
			case 'B':
				optout = true;
				verbose = false;
				break;
			case 'c':
				checkflag = true;
				break;
			case 'g':
				thebook.properties.grayscale = true;
				break;
			case 'f':
				overwrite = true;
				break;
			case 'p':
			{
				infile = optarg;
				
				if (checkflag == true) {
					cout << "OK";
					return 0;
				}
				
				if (thebook.load_document(infile)) {
					thebook.count_pages(false);
					cout << thebook.pagecount;
					return 0;
				} else {
					return 1;
				}
				
				break;
			}
			case 'i':
				infile = optarg;
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
				
				thebook.properties.quality = stoi(optarg);
				
				if (thebook.properties.quality < 75) {
					thebook.properties.quality = 75;
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
					
					thebook.printjob.rescaling = true;
					thebook.printjob.rescale_width = stod(customsize[0]);
					thebook.printjob.rescale_height = stod(customsize[1]);
					
				} else {
					thebook.printjob.rescaling = true;
					thebook.printjob.rescale_width = widthpresets[optarg];
					thebook.printjob.rescale_height = heightpresets[optarg];
				}
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
				thebook.printjob.landscapeflip = true;
				break;
			case 'k':
				thebook.properties.alterthreshold = true;
				if (!is_number(optarg)) {
					cerr << "Error: Invalid (non-numeric) threshold supplied '" << optarg << "'" << endl;
					return 1;
				}
				kparam = optarg;
				
				thebook.properties.threshold = stod(optarg) * (MaxRGB / 100); // Different on Windows vs *Nix
				break;
			case 'C':
				thebook.properties.cropflag = true;
				crop = optarg;
				break;
			case 'w':
				thebook.properties.widenflag = true;
				
				if (!is_number(optarg)) {
					cerr << "Error: Invalid (non-numeric) margin '" << optarg << "'" << endl;
					return 1;
				}
				
				thebook.properties.widenby = stoi(optarg);
				
				if (thebook.properties.widenby > 100) {
					thebook.properties.widenby = 100;
				}
				break;
			case 'D':
				thebook.properties.dividepages = true;
				break;
			case 'N':
				thebook.printjob.linear_output = true;
				break;
			case 'e':
				exportflag = true;
				thebook.printjob.previewonly = true;
				exportvalue = optarg;
				break;
			case 'a':
				thebook.properties.automargin = true;
				if (HAS_OPTIONAL_ARGUMENT) {
					if (is_number(optarg)) {
						thebook.properties.maxmargin = stoi(optarg);
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
			
	if (infile == "" && !pdfstdin) {
		if (!optout) {
			cerr << helpstring;
			return 1;
		}
		noinfile = true; // Only set if -B was specified, ie infile not needed
	}
	
	if (infile != "" && pdfstdin) {
		// Read from provided file by preference rather than STDIN if both -i and -I are supplied
		pdfstdin = false;
	}
	
	if (outfile == "" && !pdfstdout) {
		if (!optout) {
			cerr << helpstring;
			return 1;
		}
		nooutfile = true;
	}
	
	if (outfile != "" && pdfstdout) {
		pdfstdout = false;
	}
	
	if (pdfstdout) {
		
		verbose = false;
		bookthief = false;
		
		if (segflag) {
			cerr << "Warning: Printing multiple, separate PDFs to STDOUT" << endl << endl;
		}
		
	}
	

	string outstring;
	
	if (!optout) {
		outstring = (string)outfile;
	
		if (!has_ending(outstring, ".pdf") && !exportflag) {
			outstring = outstring + ".pdf";
		}
	
		if (!has_ending(outstring, ".jpeg") && exportflag) {
			outstring = outstring + ".jpeg";
		}
	
		if (file_exists(outstring) && !overwrite) {
			cerr << "Error: File '" << outstring << "' already exists!" << endl;
			return 1;
		}
	}
		
	if (thebook.properties.dividepages && segflag) {
		segsize = segsize / 2;
		if (segsize < 4) {
			segsize = 4;
		}
	}
	
	if (thebook.properties.cropflag) {
		vector<string> tmpcropvector = explode(crop, ',');
		if (tmpcropvector.size() != 4) {
			cerr << "Error: Invalid crop '" << crop << "'" << endl;
			return 1;
		}
		
		if (!is_number(tmpcropvector[0]) || !is_number(tmpcropvector[1]) || !is_number(tmpcropvector[2]) || !is_number(tmpcropvector[3])) {
			cerr << "Error: Invalid (non-numeric) crop '" << crop << "'" << endl;
			return 1;
		}
		
		thebook.properties.cropvalues[0] = stoi(tmpcropvector[0]);
		thebook.properties.cropvalues[1] = stoi(tmpcropvector[1]);
		thebook.properties.cropvalues[2] = stoi(tmpcropvector[2]);
		thebook.properties.cropvalues[3] = stoi(tmpcropvector[3]);
	
	}
	
	if (optout) {
		string xmloutput = "<settings>\n";
		if (!noinfile) {
			xmloutput = xmloutput + "<infile>" + infile + "</infile>\n";
		}
		if (!nooutfile) {
			xmloutput = xmloutput + "<outfile>" + outfile + "</outfile>\n";
		}
		if (thebook.properties.automargin) {
			xmloutput = xmloutput + "<automargin>" + to_string(thebook.properties.maxmargin) + "</automargin>\n";
		}
		if (thebook.properties.widenflag) {
			xmloutput = xmloutput + "<widen>" + to_string(thebook.properties.widenby) + "</widen>\n";
		}
		if (bookthief) {
			xmloutput = xmloutput + "<bookthief>on</bookthief>\n";
		}
		if (crop != "0,0,0,0") {
			xmloutput = xmloutput + "<crop>" + crop + "</crop>\n";
		}
		if (thebook.properties.dividepages) {
			xmloutput = xmloutput + "<divide>on</divide>\n";
		}
		if (thebook.printjob.linear_output) {
			xmloutput = xmloutput + "<linear>on</linear>\n";
		}
		if (overwrite) {
			xmloutput = xmloutput + "<overwrite>on</overwrite>\n";
		}
		if (thebook.properties.grayscale) {
			xmloutput = xmloutput + "<grayscale>on</grayscale>\n";
		}
		if (thebook.properties.alterthreshold) {
			xmloutput = xmloutput + "<threshold>" + kparam + "</threshold>\n";
		}
		if (thebook.printjob.landscapeflip) {
			xmloutput = xmloutput + "<longedge>on</longedge>\n";
		}
		if (rangeflag) {
			xmloutput = xmloutput + "<range>" + rangevalue + "</range>\n";
		}
		if (segflag) {
			xmloutput = xmloutput + "<segment>" + to_string(segsize) + "</segment>\n";
		}
		if (thebook.printjob.rescaling) {
			string stroutwidth = to_string(thebook.printjob.rescale_width);
			string stroutheight = to_string(thebook.printjob.rescale_height);
			while (has_ending(stroutwidth, "0") && !has_ending(stroutwidth, ".0")) {
				stroutwidth.pop_back();
			}
			while (has_ending(stroutheight, "0") && !has_ending(stroutheight, ".0")) {
				stroutheight.pop_back();
			}
			
			xmloutput = xmloutput + "<transform>" + stroutwidth + "x" + stroutheight + "</transform>\n";
		}
		xmloutput = xmloutput + "<minsize>" + to_string(minsize) + "</minsize>\n";
		xmloutput = xmloutput + "<quality>" + to_string(thebook.properties.quality) + "</quality>\n";
		
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
	
	if (!thebook.load_document(infilestr, pdfstdin)) {
		return 1;
	}
	
	thebook.count_pages(verbose);
	
	int pagecount = thebook.pagecount;
	
	if (!thebook.set_pages(rangeflag, rangevalue)) {
		return 1;
	}
	
	
	if (exportflag) {
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
			
			string exrange = toexport[1] + "," + toexport[0];
			if (!thebook.set_pages(true, exrange)) {
				return 1;
			}
			
		} else {
			cerr << "Error: You must select 2 pages for export" << endl;
			return 1;
		}
	}
	
	Magick::InitializeMagick(*argv);

	try {
		if (rangeflag || exportflag) {
			pagecount = thebook.selectedpages.size();
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
		
		if (checkflag) {
			cout << "OK";
			return 0;
		}
		
		int revsegsize = segsize;
		int revfinalsegsize = finalsegsize + (finalsegsize * thebook.properties.dividepages);
		
		if (segsize % 2 != 0) {
			lastpageblank = true;
			revsegsize = segsize + 1;
		}
		
		if (revfinalsegsize % 2 != 0 && !exportflag) {
			flastpageblank = true;
			revfinalsegsize = revfinalsegsize + 1;
		}
		
		if (revsegsize % 4 != 0) {
			extrablanks = true;
		}
		
		if (revfinalsegsize % 4 != 0 && !exportflag) {
			fextrablanks = true;
		}
		
		while (thisseg < segcount) {
			
			firstpage = segsize*(thisseg-1);
				
			string newname = outstring.substr(0, outstring.size()-4) + "-part" + to_string(thisseg) + ".pdf";
				
			
			thebook.printjob.finalpageblank = lastpageblank;
			thebook.printjob.extrablanks = extrablanks;
			thebook.printjob.segcount = segcount;
			thebook.printjob.thisseg = thisseg;
			thebook.printjob.startfrom = firstpage;
			
			thebook.printjob.endat = firstpage + segsize;

			thebook.load_pages(verbose, bookthief);
			
			thebook.make_booklet(verbose, bookthief);
			
			thebook.rescale(verbose, bookthief);

			thebook.pages.clear();
			if (verbose && !pdfstdout) {
				cout << endl << "Writing to file..." << endl;
			}
			if (!pdfstdout) {
				writeImages(thebook.booklet.begin(), thebook.booklet.end(), newname);
			} else {
				Magick::Blob inmemory; // writeImages() will write to this Blob
			
				for (long unsigned int i=0;i<thebook.booklet.size();i++) {
					thebook.booklet[i].magick("pdf"); // Explicitly set output type
				}
			
				writeImages(thebook.booklet.begin(), thebook.booklet.end(), &inmemory, true); // Write to Blob
			
				unsigned char* rawpdfbytes = (unsigned char*)inmemory.data(); // Access the bytes of the Blob
		
				for (long unsigned int i=0;i<inmemory.length();i++) {
					printf("%c", rawpdfbytes[i]);
				}
		
			}
				
				
			thebook.booklet.clear(); // clear memory for the sake of the user's machine
				
				
			double dpercentdone = (double)thisseg/segcount;
			int percentdone = floor(dpercentdone * 100);
			if (bookthief && !pdfstdout) {
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
		
		thebook.printjob.finalpageblank = flastpageblank;
		thebook.printjob.extrablanks = fextrablanks;
		thebook.printjob.segcount = segcount;
		thebook.printjob.thisseg = thisseg;
		thebook.printjob.startfrom = firstpage;
		
		thebook.printjob.endat = firstpage + finalsegsize;
		
		thebook.load_pages(verbose, bookthief);
		
		thebook.make_booklet(verbose, bookthief);
		
		thebook.rescale(verbose, bookthief);
		
		if (verbose && !pdfstdout) {
			cout << endl << "Writing to file..." << endl;
		}
		
		if (!pdfstdout) {
			writeImages(thebook.booklet.begin(), thebook.booklet.end(), newname);
		} else {
			Magick::Blob inmemory; // writeImages() will write to this Blob
			
			for (long unsigned int i=0;i<thebook.booklet.size();i++) {
				thebook.booklet[i].magick("pdf"); // Explicitly set output type
			}
			
			writeImages(thebook.booklet.begin(), thebook.booklet.end(), &inmemory, true); // Write to Blob
			
			unsigned char* rawpdfbytes = (unsigned char*)inmemory.data(); // Access the bytes of the Blob
		
			for (long unsigned int i=0;i<inmemory.length();i++) {
				printf("%c", rawpdfbytes[i]);
			}
		
		}
		
		if (bookthief && !pdfstdout) {
			cout << "100%" << endl;
		}
		if (!pdfstdout) {
			cout << "Done!" << endl;
		}
		return 0;
		
	}
	catch( Magick::Exception &error_ )
	{
		cerr << "Error:\n" << error_.what() << endl;
		return 1;
	}
	return 0;
}
