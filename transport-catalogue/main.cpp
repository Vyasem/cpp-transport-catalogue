#include "headers/input_reader.h"
#include "headers/json_reader.h"
#include "headers/request_handler.h"
#include "headers/stat_reader.h"
#include "headers/json.h"
#include "headers/domain.h"
#include "headers/map_renderer.h"
#include "headers/transport_catalogue.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <iomanip>

void inputTest() {
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
		reader.ClearQuery();
		fs >> queryInCount;
		fs.get();
		transport::result::StatReader Sreader(&tCatalog, fs, queryInCount);
	}
}

void jsonTest() {
	transport::catalog::TransportCatalogue tCatalog;
	transport::render::MapRenderer map;
	transport::request::RequestHandler handler(tCatalog, map);
	std::string filename = "cpp-transport-catalogue/transport-catalogue/Examples/test_json.json";
	std::fstream fs;
	fs.open(filename);
	if (fs.is_open()) {		
		transport::json_reader::JsonReader reader(handler, fs);
		reader.HandleDataBase();
		reader.HandleQuery();
		reader.Print(std::cout);
	}
}

void svgTest() {
	transport::catalog::TransportCatalogue tCatalog;
	transport::render::MapRenderer map;
	transport::request::RequestHandler handler(tCatalog, map);
	std::string filename = "cpp-transport-catalogue/transport-catalogue/Examples/svg.json";
	std::fstream fs;
	fs.open(filename);
	if (fs.is_open()) {
		transport::json_reader::JsonReader reader(handler, fs);
		reader.HandleDataBase();
		handler.DrawMap(std::cout);
	}
}

int main() {
	//inputTest();
	//jsonTest();
	svgTest();
	return 0;
}