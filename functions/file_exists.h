#ifndef STAT
#define STAT
#include <sys/stat.h>
#endif


#ifdef _WIN32
inline bool file_exists(const char *name) {
	return (_access(name, 0) != -1);
}

inline bool file_exists(std::string name) {
	return (_access(name.c_str(), 0) != -1);
}

#else

inline bool file_exists(const char *name) {
	struct stat buffer;
	return (stat (name, &buffer) == 0);
}

inline bool file_exists(std::string name) {
	struct stat buffer;
	return (stat (name.c_str(), &buffer) == 0);
}

#endif
