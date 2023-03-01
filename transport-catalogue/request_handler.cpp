#include "headers/request_handler.h"
#include "headers/map_renderer.h"
#include "headers/serialization.h"
#include <vector>
#include <unordered_map>
#include <string_view>
#include <deque>
#include <utility>
#include <optional>
#include <variant>

namespace transport {
namespace request {
	RequestHandler::RequestHandler(){}

	void RequestHandler::CreateCatalog(const std::unordered_map<std::string_view, std::pair<std::deque<std::string_view>, bool>>& buses,
		const std::unordered_map<std::string_view, std::pair<double, double>>& stops, 
		const std::vector<domain::DistanceBwStops>& stopsDistance) {

		for (const auto& [stopName, coord] : stops) {
			catalog_.AddStop(stopName, geo::Coordinates{coord.first, coord.second});
		}

		for (const auto& [busName, busInfo] : buses) {
			catalog_.AddRoute(busName, busInfo.first, busInfo.second);
		}

		for (const auto& [from, to, distance] : stopsDistance) {
			catalog_.SetDistance(from, to, distance);
		}
	}

	void RequestHandler::MakeBase() {
		Serialization ser(catalog_, map_, route_);
		CreateRoute();
		ser.Serialize(serialization_["file"]);
	}

	void RequestHandler::ProcessRequest() {
		Serialization ser(catalog_, map_, route_);
		ser.Deserialize(serialization_["file"]);
	}

	const domain::Route RequestHandler::GetRoute(const std::string_view& busName) {
		return catalog_.GetRoute(busName);
	}

	std::optional<const std::deque<std::string_view>> RequestHandler::GetStopBuses(const std::string_view& stopName) {
		try {
			const domain::Stop* stop = catalog_.StopFind(stopName);
			return catalog_.GetStopBuses(stop->name);
		}catch (...) {
			return std::nullopt;
		}
			
	}

	void RequestHandler::SetRenderSettings(std::unordered_map<std::string, domain::SettingType> settings) {
		map_.SetSettings(settings);
	}

	void RequestHandler::SetRouteSettings(std::unordered_map<std::string, double> settings) {
		route_.SetSettings(std::move(settings));
	}

	void RequestHandler::SetSerializationSettings(std::unordered_map<std::string, std::string_view> settings) {
		serialization_ = std::move(settings);
	}

	void RequestHandler::CreateRoute() {
		route_.CreateRoutes(catalog_);
	}

	const std::optional<domain::Trip>& RequestHandler::FindRoute(std::string_view from, std::string_view to) {
		const domain::Stop* fromPtr = catalog_.StopFind(from);
		const domain::Stop* toPtr = catalog_.StopFind(to);
		route_.FindRoute(fromPtr, toPtr);
		return route_.GetReadyRoute();
	}

	void RequestHandler::DrawMap(std::ostream& out) {
		map_.Draw(out, catalog_.GetAllRoutes());
	}
}
}