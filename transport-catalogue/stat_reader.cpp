#include "headers/stat_reader.h"
#include "headers/transport_catalogue.h"
#include "headers/input_reader.h"

#include <iostream>
#include <unordered_map>
#include <tuple>
#include <deque>
#include <string>

namespace transport {
	namespace result {
		StatReader::StatReader(catalog::TransportCatalogue* catalog, std::istream& input, int count) :catalog_(catalog) {
			std::string inputLine;
			for (int i = 0; i < count; ++i) {
				getline(input, inputLine);
				QueryHandle(inputLine);
			}
		}

		void StatReader::QueryHandle(std::string_view str) {
			auto [type, name] = input_read::detail::Split(str, " ");
			if (type == "Bus") {
				PrintRoute(catalog_->GetRoute(name));
			} else {
				//Что бы вывести Stop not found;
				try {
					const domain::Stop* stop = catalog_->StopFind(name);
					PrintStop(catalog_->GetStopBuses(stop->name), stop->name);
				}catch (...) {
					PrintEmptyStop(name);
				}
			}
		}

		void StatReader::PrintEmptyStop(std::string_view stopName) {
			std::cout << "Stop " << stopName << ": not found\n";
		}

		void StatReader::PrintStop(const std::deque<std::string_view>& buses, std::string_view stopName) {
			if (buses.empty()) {
				std::cout << "Stop " << stopName << ": no buses\n";
			} else{
				std::cout << "Stop " << stopName << ": buses ";
				for (std::string_view res : buses) {
					std::cout << res << " ";
				}
				std::cout << "\n";
			}
		}

		void StatReader::PrintRoute(const domain::Route result) {
			if (result.stops == 0) {
				std::cout << "Bus " << result.name << ": not found\n";
			} else {
				std::cout << "Bus " << result.name << ": " << result.stops << " stops on route, " << result.uStops << " unique stops, " << result.length << " route length, " << result.curvature << " curvature\n";
			}
		}
	}
}