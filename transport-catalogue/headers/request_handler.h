#pragma once

#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "serialization.h"
#include <vector>
#include <unordered_map>
#include <string_view>
#include <deque>
#include <utility>
#include <optional>
#include <variant>

namespace transport {
namespace request {
	class RequestHandler {
	private:
		catalog::TransportCatalogue catalog_;
		render::MapRenderer map_;
		route::Router route_;
		std::unordered_map<std::string, std::string_view> serialization_;
	public:
		explicit RequestHandler();
		void CreateCatalog(const std::unordered_map<std::string_view, std::pair<std::deque<std::string_view>, bool>>& buses, 
			const std::unordered_map<std::string_view, std::pair<double, double>>& stops, 
			const std::vector<domain::DistanceBwStops>& stopsDistance);
		void MakeBase();
		void ProcessRequest();
		const domain::Route GetRoute(const std::string_view& busName);
		std::optional<const std::deque<std::string_view>> GetStopBuses(const std::string_view& stopName);
		void SetRenderSettings(std::unordered_map<std::string, domain::SettingType> settings);
		void SetRouteSettings(std::unordered_map<std::string, double> settings);
		void SetSerializationSettings(std::unordered_map<std::string, std::string_view> settings);
		void CreateRoute();
		const std::optional<domain::Trip>& FindRoute(std::string_view from, std::string_view to);
		void DrawMap(std::ostream& out);
	};
}
}