#pragma once
#include "geo.h"
#include <string_view>
#include <deque>
#include <unordered_map>

namespace transport {
	namespace catalog {
		struct Bus {
			std::string_view name = "";
			std::deque<std::string_view> stops = {};
			bool loope = false;
			Bus() = default;
			Bus(std::string_view name_, std::deque<std::string_view> stops_, bool loope_) :name(name_), stops(stops_), loope(loope_) {};
		};

		struct Stop {
			std::string_view name = "";
			geo::Coordinates coord = {0.0, 0.0};
			Stop() = default;
			Stop(std::string_view name_, geo::Coordinates coord_) :name(name_), coord(coord_) {};
		};

		struct Route {
			std::string_view name = "";
			size_t stops = 0;
			size_t uStops = 0;
			double length = 0.0;
			double curvature= 0.0;
			Route() = default;
			Route(std::string_view name_, size_t stops_, size_t uStops_, double length_, double curvature_) :name(name_), stops(stops_), uStops(uStops_), length(length_), curvature(curvature_) {};
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
			void AddStop(std::string_view stopName, const geo::Coordinates coordinates);
			void AddRoute(std::string_view routeName, std::deque<std::string_view> stopsName, bool loope);
			void SetDistance(std::string_view stopFrom, std::string_view stopTo, int distance);
			const Bus* BusFind(std::string_view busName)const;
			const Stop* StopFind(std::string_view stopName)const;
			const Route GetRoute(std::string_view busName);
			const std::deque<std::string_view> GetStopBuses(std::string_view stopName);
			int GetDistance(Stop* stopFrom, Stop* stopTo);
		};
	}	
}