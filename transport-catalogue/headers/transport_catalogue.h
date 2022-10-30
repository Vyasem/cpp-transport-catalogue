#pragma once
#include "geo.h"
#include <string_view>
#include <deque>
#include <unordered_map>

namespace transport {
	namespace catalog {
		struct Bus {
			std::string_view name;
			std::deque<std::string_view> stops;
			bool loope;
		};

		struct Stop {
			std::string_view name;
			geo::Coordinates coord;
		};

		struct Route {
			std::string_view name;
			size_t stops;
			size_t uStops;
			double length;
			double curvature;
		};

		struct StopLengthHasher {
			size_t operator()(std::pair<Stop*, Stop*> key) const {
				return static_cast<size_t>(key.first->name.size() * key.first->coord.lat * key.first->coord.lng * key.second->name.size() * key.second->coord.lat * key.second->coord.lng);
			}
		};

		class TransportCatalogue {
		private:
			std::unordered_map<std::string_view, Bus> routes;
			std::unordered_map<std::string_view, Stop> stops;
			std::unordered_map<std::pair<Stop*, Stop*>, int, StopLengthHasher> distanceBwStops;
		public:
			explicit TransportCatalogue();
			void AddStop(std::string_view, double, double);
			void AddRoute(std::string_view, std::deque<std::string_view>, bool);
			void SetDistance(std::tuple<std::string_view, std::string_view, int>);
			const Bus* BusFind(std::string_view)const;
			const Stop* StopFind(std::string_view)const;
			const Route GetRoute(std::string_view);
			const std::deque<std::string_view> GetStopBuses(std::string_view);
			int GetDistance(std::pair<Stop*, Stop*>);
		};
	}	
}