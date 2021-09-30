inline bool starts_with(std::string haystack, std::string needle) {
	if (haystack.find(needle) == 0) {
		return true;
	}
	return false;
}
