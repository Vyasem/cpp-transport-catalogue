#pragma once
#include "geo.h"
#include "domain.h"
#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>

namespace transport {
namespace catalog {
	class TransportCatalogue {
	private:
		std::unordered_map<std::string_view, domain::Bus*> routes;
		std::unordered_map<std::string_view, domain::Stop*> stops;			
		std::deque<domain::Stop> stopStorage;
		std::deque<domain::Bus> busStorage;
		std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, int, domain::StopLengthHasher> distanceBwStops;
	public:
		explicit TransportCatalogue();
		void AddStop(std::string_view stopName, const geo::Coordinates coordinates);
		void AddRoute(std::string_view routeName, std::deque<std::string_view> stopsName, bool loope);
		void SetDistance(std::string_view stopFrom, std::string_view stopTo, int distance);
		const domain::Bus* BusFind(std::string_view busName)const;
		const domain::Stop* StopFind(std::string_view stopName)const;
		const domain::Route GetRoute(std::string_view busName);
		const std::deque<std::string_view> GetStopBuses(std::string_view stopName);
		int GetDistance(domain::Stop* stopFrom, domain::Stop* stopTo);
		std::deque<const domain::Bus*> GetAllRoutes();
	};
}	
}