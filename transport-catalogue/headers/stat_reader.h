#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <unordered_map>
#include <tuple>
#include <deque>

namespace transport {
	namespace result {
		class StatReader {
		public:
			StatReader(catalog::TransportCatalogue* catalog, std::istream& input, int count);
			void QueryHandle(std::string_view str);
			void PrintEmptyStop(std::string_view stopName);
			void PrintStop(const std::deque<std::string_view>& buses, std::string_view stopName);
			void PrintRoute(const catalog::Route result);
		private:
			catalog::TransportCatalogue* catalog_;
		};
	}	
}