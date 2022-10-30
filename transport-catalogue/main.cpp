#include "headers/input_reader.h"
#include "headers/stat_reader.h"
#include "headers/transport_catalogue.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <sstream>
#include <unordered_map>


int main() {
	transport::catalog::TransportCatalogue tCatalog;
	int queryInCount;
	std::string filename = "Examples/tsC_case3_input.txt";
	//std::string filename = "Examples/test.txt";
	std::fstream fs;

	fs.open(filename);
	fs >> queryInCount;
	fs.get();
	transport::input_read::InputReader reader(&tCatalog, fs, queryInCount);
	reader.queryHandle();
	fs >> queryInCount;
	fs.get();
	transport::result::StatReader Sreader(&tCatalog, fs, queryInCount);
	return 0;
}