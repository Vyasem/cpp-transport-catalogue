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
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, domain::StopLengthHasher> distanceBwStops;
		size_t uniqueStopCount = 0;
		size_t routesStopCount = 0;
	public:
		explicit TransportCatalogue();
		void AddStop(std::string_view stopName, const geo::Coordinates coordinates);
		void AddRoute(std::string_view routeName, std::deque<std::string_view> stopsName, bool loope);
		void SetDistance(std::string_view stopFrom, std::string_view stopTo, int distance);
		const std::deque<domain::Stop>& GetStopStorage();
		const std::deque<domain::Bus>& GetBusStorage();
		const std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, domain::StopLengthHasher>& GetAllDistance();
		const domain::Bus* BusFind(std::string_view busName)const;
		const domain::Stop* StopFind(std::string_view stopName)const;
		const domain::Route GetRoute(std::string_view busName);
		const std::deque<std::string_view> GetStopBuses(std::string_view stopName);
		double GetDistance(const domain::Stop* stopFrom, const domain::Stop* stopTo);
		std::deque<const domain::Bus*> GetAllRoutes();
		size_t GetUniqueStopCount();
	};
}	
}