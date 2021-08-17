#ifndef VECTOR
#define VECTOR
#include <vector>
#endif

#ifndef SSTREAM
#define SSTREAM
#include <sstream>
#endif

std::vector<std::string> explode(std::string const & s, char delimiter) {
	std::vector<std::string> result;
	std::istringstream iss(s);
	
	for (std::string token; std::getline(iss, token, delimiter); ) {
		result.push_back(std::move(token));
	}
	
	return result;
}
