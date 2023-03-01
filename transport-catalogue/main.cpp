#include "headers/json_reader.h"
#include "headers/request_handler.h"
#include "headers/json.h"
#include "headers/domain.h"
#include "headers/map_renderer.h"
#include "headers/transport_catalogue.h"
#include "headers/log_duration.h"


#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <iomanip>
#include <map>


using namespace std::string_view_literals;

void jsonTest() {
	transport::request::RequestHandler handler;
	std::string filename = "../cpp-transport-catalogue/transport-catalogue/Examples/test_json.json";
	std::fstream fs;
	std::ofstream out("../cpp-transport-catalogue/transport-catalogue/Examples/result.json");
	fs.open(filename);
	if (fs.is_open()) {		
		transport::json_reader::JsonReader reader(handler, fs);
		reader.HandleDataBase();
		reader.HandleQuery();
		reader.Print(out);
	}
}

void RouteTest() {
	transport::request::RequestHandler handler;
	std::string filename = "cpp-transport-catalogue/transport-catalogue/Examples/test_graph.json";
	std::fstream fs;
	fs.open(filename);
	if (fs.is_open()) {
		transport::json_reader::JsonReader reader(handler, fs);		
		reader.HandleDataBase();
		handler.CreateRoute();
		reader.HandleQuery();
		reader.Print(std::cout);
	}
}

void svgTest() {
	transport::request::RequestHandler handler;
	std::string filename = "cpp-transport-catalogue/transport-catalogue/Examples/svg.json";
	std::fstream fs;
	fs.open(filename);
	if (fs.is_open()) {
		transport::json_reader::JsonReader reader(handler, fs);
		reader.HandleDataBase();
		handler.DrawMap(std::cout);
	}
}

void PrintUsage(std::ostream& stream = std::cerr) {
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void MakeBase() {
	transport::request::RequestHandler handler;
	std::string inFilename = "D:/Project_C/Yandex_C++/Projects/Sprint9/TransportDirectory/cpp-transport-catalogue/transport-catalogue/Examples/make_base.json";	
	std::ifstream input;
	input.open(inFilename);
	if (input.is_open()) {
		transport::json_reader::JsonReader reader(handler, input);
		reader.HandleDataBase();
		handler.MakeBase();
	}
}

void ProcessRequests() {
	transport::request::RequestHandler handler;
	std::string inFilename = "D:/Project_C/Yandex_C++/Projects/Sprint9/TransportDirectory/cpp-transport-catalogue/transport-catalogue/Examples/process_requests.json";
	std::ifstream input;
	input.open(inFilename);
	if (input.is_open()) {
		transport::json_reader::JsonReader reader(handler, input);
		handler.ProcessRequest();
		reader.HandleQuery();
		std::ofstream out("process_requests_result.json");
		reader.Print(out);
	}	
}

int main(int argc, char* argv[]) {	
	//ProcessRequests();
	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	const std::string_view mode(argv[1]);

	if (mode == "make_base"sv) {		
		MakeBase();
	}else if (mode == "process_requests"sv) {		
		ProcessRequests();
	}else {
		PrintUsage();
		return 1;
	}
	return 0;
}