#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <unordered_map>
#include <tuple>
#include <deque>
#include <vector>

namespace transport{
	namespace input_read {
		namespace detail {
			std::pair<std::string_view, std::string_view> Split(std::string_view line, const std::string by);
		}
		class InputReader {
		public:
			InputReader(catalog::TransportCatalogue* catalog, std::istream& input, int lineCount);
			void HandleQuery();
			void HandleBus();
			void HandleStop(std::string_view stopName, std::string_view stopStr);
			void HandleDistance();
		private:
			std::vector<std::string> queries;
			std::vector<std::pair<std::string_view, std::string_view>> buses;
			std::vector<std::tuple<std::string_view, std::string_view, int>> stopsDistance;
			catalog::TransportCatalogue* catalog_;
		};
	}	
}