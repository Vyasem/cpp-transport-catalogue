#include "headers/json_reader.h"
#include "headers/svg.h"
#include "headers/json.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <deque>
#include <vector>
#include <string_view>
#include <utility>
#include <optional>
#include <variant>
#include <sstream>
#include <execution>

using namespace std::string_literals;

namespace transport {
	namespace json_reader {
		JsonReader::JsonReader(request::RequestHandler& handler, std::istream& input) : handler_(handler), input_(input){
			HandleStream();
		}

		void JsonReader::HandleStream() {
			json::Document jsonObject = json::Load(input_);
			data_ = jsonObject.GetRoot().AsMap().at("base_requests");
			query_ = jsonObject.GetRoot().AsMap().at("stat_requests");
			try {
				PrepareSettings(jsonObject.GetRoot().AsMap().at("render_settings"));
			}catch (...) {}
		}

		void JsonReader::PrepareSettings(const json::Node jsonSettings) {
			std::unordered_map<std::string, domain::SettingType> settings;	

			for (const auto& [key, value] : jsonSettings.AsMap()) {
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
							else {
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

		void JsonReader::HandleData() {
			for (const json::Node& item : data_->AsArray()) {
				if (item.AsMap().at("type").AsString() == "Stop") {
					stops_[item.AsMap().at("name").AsString()] = { item.AsMap().at("latitude").AsDouble(), item.AsMap().at("longitude").AsDouble() };
					try {
						for (const auto& [toStop, dictance] : item.AsMap().at("road_distances").AsMap()) {
							stopsDistance_.push_back({ item.AsMap().at("name").AsString(), toStop,  dictance.AsInt()});
						}
					}catch (...) {}
				}else{
					std::deque<std::string_view> busStops;
					for (const json::Node& busItemStop : item.AsMap().at("stops").AsArray()) {
						busStops.push_back(busItemStop.AsString());
					}
					buses_[item.AsMap().at("name").AsString()] = { busStops,  item.AsMap().at("is_roundtrip").AsBool() };
				}
			}
			handler_.CreateCatalog(buses_, stops_, stopsDistance_);	
		}

		void JsonReader::HandleQuery() {
			json::Array result;
			for (const json::Node& item : query_->AsArray()) {
				int id = item.AsMap().at("id").AsInt();
				std::string_view type = item.AsMap().at("type").AsString();				
				if (type == "Stop") {
					std::string_view name = item.AsMap().at("name").AsString();
					std::optional<const std::deque<std::string_view>> stopBus = handler_.GetStopBuses(name);
					if (stopBus.has_value()) {
						if (stopBus.value().empty()) {
							result.push_back({ json::Dict{{"request_id", id}, {"buses", json::Array{}}} });
						}
						else {
							result.push_back({ json::Dict{{"request_id", id}, {"buses", json::Array{stopBus.value().begin(), stopBus.value().end()}}} });
						}
					}else {
						result.push_back({ json::Dict{{"request_id", id}, {"error_message", "not found"s}} });
					}
				}else if (type == "Bus") {
					std::string_view name = item.AsMap().at("name").AsString();
					const domain::Route route = handler_.GetRoute(name);
					if (route.stops == 0) {
						result.push_back({ json::Dict{{"request_id", id}, {"error_message", "not found"s}} });
					}else {
						result.push_back({ json::Dict{{"request_id", id}, {"curvature", route.curvature}, {"route_length", route.length}, {"stop_count", static_cast<int>(route.stops)}, {"unique_stop_count", static_cast<int>(route.uStops)}} });
					}					
				}else if (type == "Map") {
					std::ostringstream out;
					handler_.DrawMap(out);
					result.push_back({ json::Dict{{"request_id", id}, {"map", out.str()}}});
				}
			}
			nodeResul_ = json::Node(result);
		}

		void JsonReader::Print(std::ostream& output) {
			json::PrintNode(nodeResul_, output);
		}
	}
}