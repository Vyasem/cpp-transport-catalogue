#pragma once

#include "transport_catalogue.h"
#include<iostream>
#include<unordered_map>
#include<tuple>
#include<deque>

namespace transport {
	namespace result {
		class StatReader {
		public:
			StatReader(catalog::TransportCatalogue*, std::istream&, int);
			void queryHandle(std::string_view);
			void printRoute(const catalog::Route);
			void printStop(const std::deque<std::string_view>&, std::string_view);
			void printEmptyStop(std::string_view);
		private:
			catalog::TransportCatalogue* catalog_;
		};
	}	
}