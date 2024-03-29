#include "headers/json_reader.h"
#include "headers/svg.h"
#include "headers/json.h"
#include "headers/json_builder.h"

#include <iostream>
#include <string_view>
#include <string>
#include <unordered_map>
#include <deque>
#include <vector>
#include <string_view>
#include <utility>
#include <optional>
#include <variant>
#include <sstream>
#include <filesystem>

using namespace std::string_literals;

namespace transport {
namespace json_reader {
	JsonReader::JsonReader(request::RequestHandler& handler, std::istream& input) : handler_(handler), 
		input_(input), jsonObject_(json::Load(input_)){
		PrepareSerializationSettings();
	}

	void JsonReader::HandleDataBase() {		
		for (const json::Node& item : jsonObject_.GetRoot().AsMap().at("base_requests").AsArray()) {
			const std::string& type = item.AsMap().at("type").AsString();
			const std::string& name = item.AsMap().at("name").AsString();
			if (type == "Stop") {
				double latitude = item.AsMap().at("latitude").AsDouble();
				double longtitude = item.AsMap().at("longitude").AsDouble();
				stops_[name] = { latitude, longtitude };
				try {
					const json::Dict& roadDistance = item.AsMap().at("road_distances").AsMap();
					for (const auto& [toStop, dictance] : roadDistance) {
						stopsDistance_.push_back(domain::DistanceBwStops{ name, toStop,  dictance.AsInt() });
					}
				}
				catch (...) {}
			}
			else {
				std::deque<std::string_view> busStops;
				for (const json::Node& busItemStop : item.AsMap().at("stops").AsArray()) {
					busStops.push_back(busItemStop.AsString());
				}
				buses_[name] = { busStops, item.AsMap().at("is_roundtrip").AsBool() };
			}
		}
		handler_.CreateCatalog(buses_, stops_, stopsDistance_);
		PrepareSettings();
	}

	void JsonReader::HandleQuery() {
		json::Array result;
		for (const json::Node& item : jsonObject_.GetRoot().AsMap().at("stat_requests").AsArray()) {
			std::string_view type = item.AsMap().at("type").AsString();
			if (type == "Stop") {
				HandleStopQuery(item, result);
			}
			else if (type == "Bus") {
				HandleBusQuery(item, result);
			}
			else if (type == "Map") {
				HandleMapQuery(item, result);
			}
			else if (type == "Route") {
				HandleRouteQuery(item, result);
			}
		}
		nodeResul_ = json::Node(result);
	}

	void JsonReader::PrepareSettings() {
		PrepareRenderSettings(jsonObject_.GetRoot().AsMap().at("render_settings"));
		PrepareRouteSettings(jsonObject_.GetRoot().AsMap().at("routing_settings"));
	}

	void JsonReader::PrepareSerializationSettings() {
		std::unordered_map<std::string, std::string_view> result;
		for (const auto& [key, value] : jsonObject_.GetRoot().AsMap().at("serialization_settings").AsMap()) {
			result[key] = value.AsString();
		}
		handler_.SetSerializationSettings(result);
	}

	void JsonReader::PrepareRenderSettings(const json::Node renderSettings) {
		std::unordered_map<std::string, domain::SettingType> settings;	

		for (const auto& [key, value] : renderSettings.AsMap()) {
			if (key == "color_palette") {
				std::vector<svg::Color> colorPalette;
				for (const json::Node& itemColor : value.AsArray()) {
					if (itemColor.IsString()) {
						colorPalette.push_back(itemColor.AsString());
						continue;
					}

					if (itemColor.IsArray()) {
						json::Array colors = itemColor.AsArray();
						if (colors.size() == 3) {
							colorPalette.push_back(svg::Color{ svg::Rgb{ static_cast<uint8_t>(colors[0].AsInt()),  static_cast<uint8_t>(colors[1].AsInt()),  static_cast<uint8_t>(colors[2].AsInt())} });
						}
						else if(colors.size() == 4) {
							colorPalette.push_back(svg::Color{ svg::Rgba{ static_cast<uint8_t>(colors[0].AsInt()),  static_cast<uint8_t>(colors[1].AsInt()),  static_cast<uint8_t>(colors[2].AsInt()), colors[3].AsDouble() } });
						}
					}

				}
				settings["color_palette"] = colorPalette;
			}

			if (key == "underlayer_color") {
				if (value.IsString()) {
					settings["underlayer_color"] = svg::Color{ value.AsString() };
				}else if (value.IsArray()) {
					json::Array underlayerColor = value.AsArray();
					if (underlayerColor.size() == 3) {
						settings["underlayer_color"] = svg::Color{ svg::Rgb{ static_cast<uint8_t>(underlayerColor[0].AsInt()),  static_cast<uint8_t>(underlayerColor[1].AsInt()),  static_cast<uint8_t>(underlayerColor[2].AsInt())} };
					}
					else {
						settings["underlayer_color"] = svg::Color{ svg::Rgba{ static_cast<uint8_t>(underlayerColor[0].AsInt()),  static_cast<uint8_t>(underlayerColor[1].AsInt()),  static_cast<uint8_t>(underlayerColor[2].AsInt()), underlayerColor[3].AsDouble()} };
					}
				}
			}

			if (key == "bus_label_offset" || key == "stop_label_offset") {
				if (value.IsArray()) {
					if (value.AsArray().front().IsDouble() && value.AsArray().back().IsDouble()) {
						settings[key] = std::pair<double, double>{ value.AsArray().front().AsDouble(), value.AsArray().back().AsDouble() };
					}
				}
			}

			if (value.IsDouble()) {
				settings[key] = value.AsDouble();
			}
		}			
		handler_.SetRenderSettings(settings);
	}

	void JsonReader::PrepareRouteSettings(const json::Node routeSettings) {
		std::unordered_map<std::string, double> settings;

		for (const auto& [key, value] : routeSettings.AsMap()) {
			if (value.IsDouble()) {
				settings[key] = value.AsDouble();
			}
		}
		handler_.SetRouteSettings(settings);
	}	

	void JsonReader::HandleStopQuery(const json::Node& stop, json::Array& saveConatiner){
		int id = stop.AsMap().at("id").AsInt();
		std::string_view name = stop.AsMap().at("name").AsString();
		std::optional<const std::deque<std::string_view>> stopBus = handler_.GetStopBuses(name);
		if (stopBus.has_value()) {
			if (stopBus.value().empty()) {
				saveConatiner.push_back(json::Builder{}
				.StartDict()
					.Key("request_id"s)
					.Value(id)
					.Key("buses")
					.Value(json::Array{})
				.EndDict()
				.Build());
			}else {
				saveConatiner.push_back(json::Builder{}
				.StartDict()
					.Key("request_id"s)
					.Value(id)
					.Key("buses")
					.Value(json::Array{stopBus.value().begin(), stopBus.value().end()})
				.EndDict()
				.Build());
			}
		}else {			
			saveConatiner.push_back(json::Builder{}
			.StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("error_message"s)
				.Value("not found"s)
			.EndDict()
			.Build());
		}
	}

	void JsonReader::HandleBusQuery(const json::Node& bus, json::Array& saveConatiner){
		int id = bus.AsMap().at("id").AsInt();
		std::string_view name = bus.AsMap().at("name").AsString();
		const domain::Route route = handler_.GetRoute(name);
		if (route.stops == 0) {
			saveConatiner.push_back(json::Builder{}
			.StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("error_message"s)
				.Value("not found"s)
			.EndDict()
			.Build());
		}else {
			saveConatiner.push_back(json::Builder{}
			.StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("curvature"s)
				.Value(route.curvature)
				.Key("route_length"s)
				.Value(route.length)
				.Key("stop_count"s)
				.Value(static_cast<int>(route.stops))
				.Key("unique_stop_count"s)
				.Value(static_cast<int>(route.uStops))
			.EndDict()
			.Build());			
		}
	}

	void JsonReader::HandleMapQuery(const json::Node& map, json::Array& saveConatiner){
		int id = map.AsMap().at("id").AsInt();
		std::ostringstream out;
		handler_.DrawMap(out);
		saveConatiner.push_back(json::Builder{}
		.StartDict()
			.Key("request_id"s)
			.Value(id)
			.Key("map")
			.Value(out.str())
		.EndDict()
		.Build());
	}

	void JsonReader::HandleRouteQuery(const json::Node& route, json::Array& saveConatiner) {
		int id = route.AsMap().at("id").AsInt();
		std::string_view from = route.AsMap().at("from").AsString();
		std::string_view to = route.AsMap().at("to").AsString();
		const std::optional<domain::Trip>& result = handler_.FindRoute(from, to);
		if (result.has_value()) {
			json::Array routeItems;
			const domain::Trip& trip = result.value();
			for (const domain::TripAction item : trip.items) {
				json::Builder builder = json::Builder{};
				auto itemBuilder = builder.StartDict();
				if (item.type == "Wait") {
					itemBuilder.Key("stop_name"s).Value(std::string(item.stopBusName));
				}else {
					itemBuilder.Key("bus"s)
						.Value(std::string(item.stopBusName))
						.Key("span_count"s)
						.Value(item.spanCount);
				}
				itemBuilder.Key("time"s)
					.Value(item.time)
					.Key("type"s)
					.Value(item.type)
				.EndDict();
				
				json::Node readyResult = builder.Build();
				routeItems.push_back(readyResult);
			}
			saveConatiner.push_back(json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("total_time"s)
				.Value(trip.totalTime)
				.Key("items"s)
				.Value(routeItems)
				.EndDict()
			.Build());
		}else {
			saveConatiner.push_back(json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("error_message"s)
				.Value("not found"s)
			.EndDict()
			.Build());			
		}
	}

	void JsonReader::Print(std::ostream& output) {
		json::PrintNode(nodeResul_, output);
	}

	std::string_view JsonReader::GetSerializePath() {		
		return jsonObject_.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString();
	}
}
}