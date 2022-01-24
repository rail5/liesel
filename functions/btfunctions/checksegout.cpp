using namespace std;

bool checksegout(string outstring, int segments, bool force = false, bool pdfstdout = false) {

	if (force == true || pdfstdout == true) {
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
