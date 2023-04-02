#ifdef _WIN32
inline bool file_exists(const char *name) {
	return (_access(name, 0) != -1);
}

inline bool file_exists(std::string name) {
	return (_access(name.c_str(), 0) != -1);
}

#else

inline bool file_exists(const char *name) {
	return (access(name, F_OK) != -1);
}

inline bool file_exists(std::string name) {
	return (access(name.c_str(), F_OK) != -1);
}

#endif
