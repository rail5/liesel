std::string lowerthecase(std::string input) {
	std::transform(input.begin(), input.end(), input.begin(), ::tolower);
	return input;
}

inline bool has_ending (std::string const &haystack, std::string const &needle) {
	std::string fullString = lowerthecase(haystack);
	std::string ending = lowerthecase(needle);

	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}
