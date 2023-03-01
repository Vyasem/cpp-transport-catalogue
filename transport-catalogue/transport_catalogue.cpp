#include "headers/domain.h"
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
		size_t stopId = stopStorage.size();
		stopStorage.push_back(domain::Stop{std::string(stopName), coordinates, stopId });
		stops[stopStorage.back().name] = &stopStorage.back();
		++uniqueStopCount;
	}

	void TransportCatalogue::AddRoute(std::string_view routeName, std::deque<std::string_view> stopsName, bool loope) {
		std::deque<const domain::Stop*> newStops;
		for (auto it = stopsName.begin(); it != stopsName.end(); ++it) {
			newStops.push_back(StopFind(*it));
		}
		size_t BusId = busStorage.size();
		routesStopCount += newStops.size();
		busStorage.push_back(domain::Bus{ std::string(routeName) , std::move(newStops), loope, BusId});
		routes[busStorage.back().name] = &busStorage.back();
	}

	void TransportCatalogue::SetDistance(std::string_view stopFrom, std::string_view stopTo, int distance) {
		if (stops.find(stopFrom) != stops.end() && stops.find(stopTo) != stops.end()) {
			distanceBwStops[{stops.at(stopFrom), stops.at(stopTo)}] = distance;
		}
	}

	const std::deque<domain::Stop>& TransportCatalogue::GetStopStorage() {
		return stopStorage;
	}

	const std::deque<domain::Bus>& TransportCatalogue::GetBusStorage() {
		return busStorage;
	}

	const std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, domain::StopLengthHasher>& TransportCatalogue::GetAllDistance() {
		return distanceBwStops;
	}

	const domain::Bus* TransportCatalogue::BusFind(std::string_view busName)const {
		return routes.at(busName);
	}

	const domain::Stop* TransportCatalogue::StopFind(std::string_view stopName)const {
		return stops.at(stopName);
	}

	const domain::Route TransportCatalogue::GetRoute(std::string_view busName) {
		try {
			const domain::Bus* busInfo = BusFind(busName);
			domain::Route result;
			result.name = busInfo->name;
			result.stops = busInfo->stops.size();
			result.length = 0;
			result.curvature = 0;

			for (auto it = busInfo->stops.begin(); it != busInfo->stops.end() - 1; ++it) {
				double curvature = geo::ComputeDistance((*it)->coord, (*(it + 1))->coord);
				double length = GetDistance(stops.at((*it)->name), stops.at((*(it + 1))->name));
				result.length += length;
				if (length == 0) {
					result.length += curvature;
				}
				result.curvature += curvature;
			}

			if (!busInfo->loope) {
				for (auto it = (busInfo->stops.end() - 1); it != busInfo->stops.begin(); --it) {
					double curvature = geo::ComputeDistance((*it)->coord, (*(it - 1))->coord);
					double length = GetDistance(stops.at((*it)->name), stops.at((*(it - 1))->name));
					result.length += length;
					if (length == 0) {
						result.length += curvature;
					}
					result.curvature += curvature;
				}
				result.stops = busInfo->stops.size() * 2 - 1;
			}

			std::deque<const domain::Stop*> st = busInfo->stops;
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
			auto findRes = std::find_if(busInfo->stops.begin(), busInfo->stops.end(), [stopName](const domain::Stop* stop) {
				return stop->name == stopName;
			});
			if (findRes != busInfo->stops.end()) {
				result.push_back(busname);
			}
		}
		std::sort(result.begin(), result.end());
		return result;
	}

	double TransportCatalogue::GetDistance(const domain::Stop* stopFrom, const domain::Stop* stopTo) {
		if (distanceBwStops.find({stopFrom, stopTo}) != distanceBwStops.end()) {
			return distanceBwStops.at({stopFrom, stopTo});
		}

		if (distanceBwStops.find({stopTo, stopFrom}) != distanceBwStops.end()) {
			return distanceBwStops.at({stopTo, stopFrom});
		}
		return 0;
	}

	std::deque<const domain::Bus*> TransportCatalogue::GetAllRoutes() {
		std::deque<const domain::Bus*> result;
		for (const auto& [name, bus] : routes) {
			if (bus->stops.size() > 0) {
				result.push_back(bus);
			}
		}
		std::sort(result.begin(), result.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {
			return lhs->name < rhs->name;
		});
		return result;
	}

	size_t TransportCatalogue::GetUniqueStopCount() {
		return uniqueStopCount;
	}
}
}