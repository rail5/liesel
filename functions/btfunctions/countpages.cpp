using namespace std;

int countpages(string infile, bool verbose, bool checkflag, bool pdfstdin) {

	poppler::document* document;
	
	if (!pdfstdin) {
		document = poppler::document::load_from_file(infile.c_str());
	} else {
		document = poppler::document::load_from_raw_data(infile.c_str(), infile.size());
	}
	if (verbose == true && checkflag == false) {
		cout << "Pages counted: " << document->pages() << "\n";
	}
	
	return document->pages();
}
