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
	//std::string filename = "cpp-transport-catalogue/transport-catalogue/Examples/tsC_case3_input.txt";
	std::string filename = "cpp-transport-catalogue/transport-catalogue/Examples/test.txt";
	std::fstream fs;

	fs.open(filename);
	if (fs.is_open()) {
		fs >> queryInCount;
		fs.get();
		transport::input_read::InputReader reader(&tCatalog, fs, queryInCount);
		reader.HandleQuery();
		fs >> queryInCount;
		fs.get();
		transport::result::StatReader Sreader(&tCatalog, fs, queryInCount);
	}	
	return 0;
}