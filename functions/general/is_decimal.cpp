inline bool is_decimal(const std::string& s)
{
	size_t n = std::count(s.begin(), s.end(), '.');
	if (n > 1 || starts_with(s, ".") || has_ending(s, ".")) {
		return false;
	}
	return s.find_first_not_of("0123456789.") == std::string::npos;
}
