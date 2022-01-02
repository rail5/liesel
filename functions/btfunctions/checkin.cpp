using namespace std;

bool checkin(char* infile, bool speak) {
	// Infile sanity checks written as an isolated function to be used in both -i and -p cases
	if (!file_exists(infile)) {
		if (!speak) {
			return false;
		}
		cout << "Error: File '" << infile << "' not found" << endl;
		return false;
	}
	
	if (!has_ending((string)infile, ".pdf")) {
		if (!speak) {
			 return false;
		}
		cout << "Error: At this stage, Liesel only supports PDFs\nPlease look for a future version to support other formats" << endl;
		return false;
	}
	
	return true;
}

