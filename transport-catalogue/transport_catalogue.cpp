#include "headers/transport_catalogue.h"
#include "headers/geo.h"

#include <string_view>
#include <deque>
#include <unordered_map>
#include <algorithm>

namespace transport {
	namespace catalog {
		TransportCatalogue::TransportCatalogue() {}

		void TransportCatalogue::AddStop(std::string_view stopName, double latitude, double longitude) {
			Stop stop{
				stopName,
				{latitude, longitude}
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

		void TransportCatalogue::SetDistance(std::tuple<std::string_view, std::string_view, int> distance) {
			auto [stopFrom, stopTo, dis] = distance;
			if (stops.find(stopFrom) != stops.end() && stops.find(stopTo) != stops.end()) {
				distanceBwStops[{&stops.at(stopFrom), & stops.at(stopTo)}] = dis;
			}
		}

		const Bus* TransportCatalogue::BusFind(std::string_view busName)const {
			try {
				return &(routes.at(busName));
			}
			catch (...) {
				return {};
			}
		}

		const Stop* TransportCatalogue::StopFind(std::string_view stopName)const {
			try {
				return &stops.at(stopName);
			}
			catch (...) {
				return {};
			}
		}

		const Route TransportCatalogue::GetRoute(std::string_view busName) {
			const Bus* busInfo = BusFind(busName);
			if (busInfo == NULL) {
				return { busName, 0, 0, 0, 0 };
			}
			Route result;
			result.name = busInfo->name;
			result.stops = busInfo->stops.size();
			result.length = 0;
			result.curvature = 0;

			for (auto it = busInfo->stops.begin(); it != busInfo->stops.end() - 1; ++it) {
				double curvature = geo::ComputeDistance(StopFind(*it)->coord, StopFind(*(it + 1))->coord);
				double length = GetDistance({ &stops.at(*it),&stops.at(*(it + 1)) });
				result.length += length;
				if (length == 0) {
					result.length += curvature;
				}
				result.curvature += curvature;

			}

			if (!busInfo->loope) {
				for (auto it = busInfo->stops.end() - 1; it != busInfo->stops.begin(); --it) {
					double curvature = geo::ComputeDistance(StopFind(*it)->coord, StopFind(*(it - 1))->coord);
					double length = GetDistance({ &stops.at(*it),&stops.at(*(it - 1)) });
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

		int TransportCatalogue::GetDistance(std::pair<Stop*, Stop*> dist) {
			if (distanceBwStops.find(dist) != distanceBwStops.end()) {
				return distanceBwStops.at(dist);
			}

			if (distanceBwStops.find({ dist.second, dist.first }) != distanceBwStops.end()) {
				return distanceBwStops.at({ dist.second, dist.first });
			}
			return 0;
		}
	}
}