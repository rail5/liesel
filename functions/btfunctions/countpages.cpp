using namespace std;

int countpages(string infile, bool verbose, bool checkflag) {
	poppler::document* document = poppler::document::load_from_file(infile.c_str());
	
	if (verbose == true && checkflag == false) {
		cout << "Pages counted: " << document->pages() << "\n";
	}
	
	return document->pages();
}
