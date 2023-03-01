#pragma once
#include "json.h"
#include "request_handler.h"
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <deque>
#include <vector>
#include <string_view>
#include <utility>
#include <filesystem>

namespace transport {
namespace json_reader {
	class JsonReader {
	private:
		request::RequestHandler& handler_;
		std::istream& input_;
		json::Document jsonObject_;
		json::Node nodeResul_;
		std::unordered_map<std::string_view, std::pair<std::deque<std::string_view>, bool>> buses_;
		std::unordered_map<std::string_view, std::pair<double, double>> stops_;
		std::vector<domain::DistanceBwStops> stopsDistance_;		
		void PrepareSettings();
		void PrepareSerializationSettings();
		void PrepareRenderSettings(const json::Node renderSettings);
		void PrepareRouteSettings(const json::Node routeSettings);
		void HandleStopQuery(const json::Node& stop, json::Array& saveConatiner);
		void HandleBusQuery(const json::Node& bus, json::Array& saveConatiner);
		void HandleMapQuery(const json::Node& map, json::Array& saveConatiner);
		void HandleRouteQuery(const json::Node& route, json::Array& saveConatiner);		
	public:
		JsonReader(request::RequestHandler& handler, std::istream& input);			
		void HandleDataBase();
		void HandleQuery();
		void Print(std::ostream& output);
		std::string_view GetSerializePath();
	};
}
}