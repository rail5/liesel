using namespace std;

bool iswritable(string filename) {

	#ifdef _WIN32
	return true; // all directories in Windows have write access
	#else

	string path = filename.substr(0, filename.find_last_of("/\\") + 1);
	
	if (path == "") {
		path = filesystem::current_path();
	}
	
	return (access(path.c_str(), W_OK) == 0);
	#endif
}
