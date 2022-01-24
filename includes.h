/* PreReq C++ & C includes */
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <sstream>
#include <filesystem>
#include <memory>
#include <vector>
#include <sstream>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <string>

/* Liesel dependencies: GraphicsMagick + Poppler */
#include <Magick++.h>

#include <poppler/cpp/poppler-global.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page-renderer.h>

/* Custom/general includes */
#include "functions/general/explode.cpp" // Explodes string into vector<string> using arbitrary given delimiter (ie, "8.5x11" exploded by delimiter 'x' returns vector[0] = "8.5", vector[1] = "11")
#include "functions/general/has_ending.cpp" // bool STR ends with SUBSTR
#include "functions/general/starts_with.cpp" // bool STR starts with SUBSTR
#include "functions/general/is_number.cpp" // bool STR consists of only digits
#include "functions/general/is_decimal.cpp" // bool STR is either an integer or a proper decimal number (ie, strings "3" and "8.5" pass, strings "abc" and "8.5.3" fail)
#include "functions/general/file_exists.cpp" // bool Given file exists
#include "functions/general/iswritable.cpp" // bool Path is writable by effective UID

/* Liesel-specific includes */
#include "functions/btfunctions/countpages.cpp" // int Total numbers of pages (first page = 1) of given PDF
#include "functions/btfunctions/progresscounter.cpp" // void COUTs current progress percentage
#include "functions/btfunctions/loadpages.cpp" // vector<Magick::Image> pages from PDF
#include "functions/btfunctions/makepamphlet.cpp" // vector<Magick::Image> base booklet
#include "functions/btfunctions/mayberescale.cpp" // vector<Magick::Image> rescaled (option -t) final booklet
#include "functions/btfunctions/checkin.cpp" // bool Input file is sane (file exists, is a PDF)
#include "functions/btfunctions/checksegout.cpp" // bool Segmented output files don't already exist (unless -f is specified)
