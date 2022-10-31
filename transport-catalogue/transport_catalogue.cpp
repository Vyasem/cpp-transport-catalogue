#include "headers/transport_catalogue.h"
#include "headers/geo.h"

#include <string_view>
#include <deque>
#include <unordered_map>
#include <algorithm>

namespace transport {
	namespace catalog {
		TransportCatalogue::TransportCatalogue() {}

		void TransportCatalogue::AddStop(std::string_view stopName, geo::Coordinates coordinates) {
			Stop stop{
				stopName,
				coordinates
			};
			stops[stopName] = stop;
		}

		void TransportCatalogue::AddRoute(std::string_view routeName, std::deque<std::string_view> stopsName, bool loope) {
			Bus bus{
				routeName,
				stopsName,
				loope
			};
			routes[routeName] = bus;
		}

		void TransportCatalogue::SetDistance(std::string_view stopFrom, std::string_view stopTo, int distance) {
			if (stops.find(stopFrom) != stops.end() && stops.find(stopTo) != stops.end()) {
				distanceBwStops[{&stops.at(stopFrom), & stops.at(stopTo)}] = distance;
			}
		}

		const Bus* TransportCatalogue::BusFind(std::string_view busName)const {
			return &(routes.at(busName));
		}

		const Stop* TransportCatalogue::StopFind(std::string_view stopName)const {
			return &stops.at(stopName);
		}

		const Route TransportCatalogue::GetRoute(std::string_view busName) {
			try {
				const Bus* busInfo = BusFind(busName);
				Route result;
				result.name = busInfo->name;
				result.stops = busInfo->stops.size();
				result.length = 0;
				result.curvature = 0;

				for (auto it = busInfo->stops.begin(); it != busInfo->stops.end() - 1; ++it) {
					double curvature = geo::ComputeDistance(StopFind(*it)->coord, StopFind(*(it + 1))->coord);
					double length = GetDistance(&stops.at(*it), &stops.at(*(it + 1)));
					result.length += length;
					if (length == 0) {
						result.length += curvature;
					}
					result.curvature += curvature;

				}

				if (!busInfo->loope) {
					for (auto it = busInfo->stops.end() - 1; it != busInfo->stops.begin(); --it) {
						double curvature = geo::ComputeDistance(StopFind(*it)->coord, StopFind(*(it - 1))->coord);
						double length = GetDistance(&stops.at(*it), &stops.at(*(it - 1)));
						result.length += length;
						if (length == 0) {
							result.length += curvature;
						}
						result.curvature += curvature;
					}
					result.stops = busInfo->stops.size() * 2 - 1;
				}

				std::deque<std::string_view> st = busInfo->stops;
				std::sort(st.begin(), st.end());
				auto last = std::unique(st.begin(), st.end());
				result.uStops = last - st.begin();
				result.curvature = result.length / result.curvature;
				return result;
			}catch (...) {
				return { busName, 0, 0, 0, 0 };
			}			
		}

		const std::deque<std::string_view> TransportCatalogue::GetStopBuses(std::string_view stopName) {
			std::deque<std::string_view> result;
			for (auto& [busname, busInfo] : routes) {
				auto findRes = std::find(busInfo.stops.begin(), busInfo.stops.end(), stopName);
				if (findRes != busInfo.stops.end()) {
					result.push_back(busname);
				}
			}
			std::sort(result.begin(), result.end());
			return result;
		}

		int TransportCatalogue::GetDistance(Stop* stopFrom, Stop* stopTo) {
			if (distanceBwStops.find({stopFrom, stopTo}) != distanceBwStops.end()) {
				return distanceBwStops.at({stopFrom, stopTo});
			}

			if (distanceBwStops.find({stopTo, stopFrom}) != distanceBwStops.end()) {
				return distanceBwStops.at({stopTo, stopFrom});
			}
			return 0;
		}
	}
}