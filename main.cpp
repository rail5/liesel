/**
 * Liesel: Prepares PDFs to be home-printed as booklets 
 * Copyright (C) 2021-2025 rail5
 */

#include <iostream>
#include <string>
#include <array>
#include <vector>

#include <unistd.h>
#include <getopt.h>
#include <cstring>

#include <Magick++.h>
#include <poppler/cpp/poppler-global.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <hpdf.h>

#define HAS_OPTIONAL_ARGUMENT \
	((optarg == NULL && optind < argc && argv[optind][0] != '-') \
	? static_cast<bool>(optarg = argv[optind++]) \
	: (optarg != NULL))

int main(int argc, char* argv[]) {
	const std::string version_string = "12.0.0";

	const std::string help_string = 
		"Liesel "
		+ version_string +
		"\n"
		"Usage: liesel [options]\n"
		"Options:\n"
		"CONTROL OPTIONS\n"
		"  -i, --input <file>       Input PDF file (if '-', read from stdin_\n"
		"  -o, --output <file>      Output PDF file (if '-', write to stdout)\n"
		"  -e, --export             Export preview image (do not build pdf)\n"
		"  -p, --pages              Count pages of input PDF and exit\n"
		"  -v, --verbose            Verbose output\n"
		"  -b, --progress           Show progress\n"
		"  -c, --check              Check validity of command line options (do not run)\n"
		"  -f, --force              Force overwrite output files\n"
		"PDF PROCESSING OPTIONS\n"
		"  -r, --range <range>      Process only specified range\n"
		"                            e.g.: 1-3,5,7-9\n"
		"  -s, --segment <size>     Output multiple PDFs in segments of given size\n"
		"                             e.g.: -s 40 gives 40 pages per segment\n"
		"  -m, --minsize <size>     Minimum segment size (default: 4)\n"
		"  -g, --grayscale          Convert to grayscale\n"
		"  -d, --density <ppi>      Image PPI (default: 100)\n"
		"  -t, --transform <size>   Rescale PDF to paper size\n"
		"                             e.g.: -t 8.5x11\n"
		"  -l, --landscape          For duplex printers on landscape mode\n"
		"  -k, --threshold <value>  Convert to pure black-and-white (not grayscale)\n"
		"                             e.g.: -k 50 will convert each pixel with\n"
		"                             brightness >50% to white, and <50% to black\n"
		"  -C, --crop <l,r,t,b>     Crop pages\n"
		"                             e.g.: -C 10,20,30,40\n"
		"                             Crops 10% from left, 20% from top,\n"
		"                             30% from right, and 40% from bottom\n"
		"  -w, --widen <amount>     Widen center margins between pages by <amount>\n"
		"  -a, --auto-widen [max]   Automatically widen center margins (optional max)\n"
		"  -D, --divide             Separate left/right halves of each page\n"
		"  -N, --no-booklet         No booklet mode\n"
		"INFO OPTIONS\n"
		"  -q, --info               Print program info\n"
		"  -V, --version            Print version number\n"
		"  -h, --help               Print this help message";

	std::string input_file;
	std::string output_file;
	
	std::string binary_input;

	bool outputting_to_stdout = false;
	bool verbose = false;
	bool output_progress = false;

	bool export_preview = false;
	bool allow_overwrite = false;
	
	// Getopt
	int c;
	opterr = 0;
	int option_index = 0;

	static struct option long_options[] = 
	{
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
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

		{"export", no_argument, 0, 'e'},
		{"grayscale", no_argument, 0, 'g'},
		{"greyscale", no_argument, 0, 'g'},
		{"force", no_argument, 0, 'f'},
		{"verbose", no_argument, 0, 'v'},
		{"progress", no_argument, 0, 'b'},
		{"pages", no_argument, 0, 'p'},
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

	while ((c = getopt_long(argc, argv, "a::ci:o:r:s:pd:t:m:ek:C:w:ghqfvblDNVB", long_options, &option_index)) != -1) {
		switch (c) {
			case 'h':
				std::cout << help_string << std::endl;
				return 0;
				break;
			case 'i':
				// Input file
				input_file = optarg;
				break;
			case 'o':
				// Output file
				output_file = optarg;
				break;
			case 'e':
				// Export preview
				export_preview = true;
				break;
			case 'r':
				// Range

			case 's':
				// Segment

			case 'p':
				// Pages

			case 'd':
				// Density

			case 't':
				// Transform

			case 'm':
				// Min size

			case 'k':
				// Threshold

			case 'C':
				// Crop

			case 'w':
				// Widen

			case 'g':
			case 'v':
			case 'b':
			case 'l':
			case 'D':
			case 'N':
			case 'f':
			case 'q':
			case 'V':
				std::cout << "Option not implemented yet." << std::endl;
				return 1;
			case '?':
				switch (optopt) {
					case 'i':
					case 'o':
					case 'r':
					case 's':
					case 'p':
					case 'd':
					case 'm':
					case 'k':
					case 'C':
					case 'w':
					case 't':
						std::cerr << "Option -" << static_cast<char>(optopt) << " requires an argument." << std::endl;
						break;
					default:
						if (isprint(optopt)) {
							std::cerr << "Unknown option -" << static_cast<char>(optopt) << std::endl;
						} else {
							std::cerr << "Unknown option character " << optopt << std::endl;
						}
						break;
				}
				return 1;
		}
	}

	// Input file checks
	if (input_file.empty()) {
		std::cerr << "Input file is required." << std::endl;
		return 1;
	}
	
	// Input file being '-' means stdin
	if (input_file == "-") {
		const size_t STDIN_BUFFER_SIZE = 1024;

		try {
			freopen(nullptr, "rb", stdin);
			if (ferror(stdin)) {
				throw std::runtime_error(strerror(errno));
			}

			size_t len;
			std::array<char, STDIN_BUFFER_SIZE> buf;
			std::vector<char> stdinput;

			while ((len = fread(buf.data(), sizeof(buf[0]), buf.size(), stdin)) > 0) {
				if (ferror(stdin) && !feof(stdin)) {
					throw std::runtime_error(strerror(errno));
				}

				stdinput.insert(stdinput.end(), buf.data(), buf.data() + len);
			}

			for (uint64_t i = 0; i < stdinput.size(); ++i) {
				binary_input += stdinput[i];
			}
		} catch (const std::exception& e) {
			std::cerr << "Error reading from stdin: " << e.what() << std::endl;
			return 1;
		}
	}

	if (output_file.empty()) {
		std::cerr << "Output file is required." << std::endl;
		return 1;
	}

	if (output_file == "-") {
		// If outputting to stdout, don't clutter
		outputting_to_stdout = true;
		verbose = false;
		output_progress = false;
	}

	if (!output_file.ends_with(".pdf") && !export_preview) {
		output_file += ".pdf";
	} else if (!output_file.ends_with(".jpeg") && export_preview) {
		output_file += ".jpeg";
	}

	if (!allow_overwrite && access(output_file.c_str(), F_OK) != -1) {
		std::cerr << "Output file already exists. Use --force to overwrite." << std::endl;
		return 1;
	}
}
