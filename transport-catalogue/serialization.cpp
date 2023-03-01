#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <string_view>
#include <variant>
#include <vector>
#include <utility>

#include "headers/serialization.h"
#include "headers/transport_catalogue.h"
#include "headers/domain.h"
#include "headers/svg.h"
#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <graph.pb.h>
#include <transport_router.pb.h>

using namespace std::string_literals;

Serialization::Serialization(transport::catalog::TransportCatalogue& catalog, transport::render::MapRenderer& map, 
	transport::route::Router& route) : catalog_(catalog), map_(map), route_(route) {};

void Serialization::Serialize(const std::filesystem::path& path){
	std::ofstream out(path, std::ios::binary);	
	serialize::SeriliazeBlock all;
	*all.mutable_catalog() = SerializeCatalog();
	*all.mutable_map() = SerializeMap();
	*all.mutable_router() = SerializeRouter();
	all.SerializePartialToOstream(&out);
}

void Serialization::Deserialize(const std::filesystem::path& path){
	std::ifstream in_file(path, std::ios::binary);
	serialize::SeriliazeBlock all;	
	if (all.ParseFromIstream(&in_file)) {
		DeserializeCatalog(all.catalog());
		DeserializeMap(all.map());
		DeserializeRouter(all.router());
	}	
}

serialize::TransportCatalogue Serialization::SerializeCatalog(){
	serialize::TransportCatalogue scatalog;
	const std::deque<transport::domain::Stop>& stops = catalog_.GetStopStorage();
	const std::deque<transport::domain::Bus>& buses = catalog_.GetBusStorage();
	const auto & distance = catalog_.GetAllDistance();

	for (const transport::domain::Stop& itemStop : stops) {
		serialize::Stop protoStop;
		serialize::Coordinates coord;
		coord.set_lat(itemStop.coord.lat);
		coord.set_lng(itemStop.coord.lng);
		protoStop.set_name(itemStop.name);
		protoStop.set_id(static_cast<int32_t>(itemStop.id));
		*protoStop.mutable_coordinates() = coord;
		*scatalog.add_stopstorage() = protoStop;
	}

	for (const transport::domain::Bus& itemBus : buses) {
		serialize::Bus protoBus;
		protoBus.set_name(itemBus.name);
		protoBus.set_loop(itemBus.loope);
		for (const transport::domain::Stop *itemStop : itemBus.stops) {
			protoBus.add_stops(static_cast<int32_t>(itemStop->id));
		}
		protoBus.set_id(static_cast<int32_t>(itemBus.id));
		*scatalog.add_busstorage() = protoBus;
	}

	for (const auto& [fromTo, length] : distance) {
		serialize::DistanceBwStops protoDistance;
		protoDistance.set_fromstop(static_cast<int32_t>(fromTo.first->id));
		protoDistance.set_tostop(static_cast<int32_t>(fromTo.second->id));
		protoDistance.set_distance(length);
		*scatalog.add_distance() = protoDistance;
	}

	return scatalog;
}

void Serialization::DeserializeCatalog(serialize::TransportCatalogue scatalog){
	const auto stopStorage = scatalog.stopstorage();
	for (auto it = stopStorage.begin(); it != stopStorage.end(); ++it) {
		catalog_.AddStop(it->name(), { it->coordinates().lat(), it->coordinates().lng() });
		stopIt[it->id()] = catalog_.StopFind(it->name());
	}

	const auto busStorage = scatalog.busstorage();
	for (auto it = busStorage.begin(); it != busStorage.end(); ++it) {
		std::deque<std::string_view> stopsName;
		const auto busStops = it->stops();
		for (auto sit = busStops.begin(); sit != busStops.end(); ++sit) {
			stopsName.push_back(stopIt.at(*sit)->name);
		}
		catalog_.AddRoute(it->name(), stopsName, it->loop());	
		busIt[it->id()] = catalog_.BusFind(it->name());
	}

	const auto distance = scatalog.distance();
	for (auto it = distance.begin(); it != distance.end(); ++it) {
		catalog_.SetDistance(stopIt.at(it->fromstop())->name, stopIt.at(it->tostop())->name, it->distance());
	}
}

serialize::MapRenderer Serialization::SerializeMap() {
	serialize::MapRenderer smap;
	const std::unordered_map<std::string, transport::domain::SettingType>& settings = map_.GetSettings();

	serialize::BusLableOffset b_lb;
	std::pair<double, double> bus_lable_offset = std::get<std::pair<double, double>>(settings.at("bus_label_offset"));
	b_lb.set_dx(bus_lable_offset.first);
	b_lb.set_dy(bus_lable_offset.second);

	serialize::StopLabelOffset s_lb;
	std::pair<double, double> stop_lable_offset = std::get<std::pair<double, double>>(settings.at("stop_label_offset"));
	s_lb.set_dx(stop_lable_offset.first);
	s_lb.set_dy(stop_lable_offset.second);

	svg::Color textColor = std::get<svg::Color>(settings.at("underlayer_color"));
	AddUnderLayerColor(smap, textColor);	

	std::vector<svg::Color> colorPalette = std::get<std::vector<svg::Color>>(settings.at("color_palette"));	
	AddColorPallete(smap, colorPalette);	

	smap.set_width(std::get<double>(settings.at("width")));
	smap.set_height(std::get<double>(settings.at("height")));
	smap.set_padding(std::get<double>(settings.at("padding")));
	smap.set_line_width(std::get<double>(settings.at("line_width")));
	smap.set_stop_radius(std::get<double>(settings.at("stop_radius")));
	smap.set_bus_label_font_size(static_cast<int>(std::get<double>(settings.at("bus_label_font_size"))));
	*smap.mutable_bus_label_offset() = std::move(b_lb);
	*smap.mutable_stop_label_offset() = std::move(s_lb);
	smap.set_stop_label_font_size(static_cast<int>(std::get<double>(settings.at("stop_label_font_size"))));
	smap.set_underlayer_width(std::get<double>(settings.at("underlayer_width")));

	return smap;
}

serialize::TransportRouter Serialization::SerializeRouter() {
	serialize::TransportRouter ser_router;
	serialize::Graph ser_graph = SerializeGraph(route_.GetGraph());
	for (const auto& [key, value] : route_.GetSettings()) {
		(*ser_router.mutable_settings())[key] = value;
	}
	
	*ser_router.mutable_graph() = std::move(ser_graph);	
	return ser_router;
}

serialize::Graph Serialization::SerializeGraph(const graph::DirectedWeightedGraph<double>& graph) {
	serialize::Graph ser_graph;
	size_t graphSize = graph.GetVertexCount();
	serialize::EdgeList ser_edges_list;

	for (size_t i = 0; i < graphSize; ++i) {		
		auto edgesId = graph.GetIncidentEdges(i);

		for (auto it = edgesId.begin(); it != edgesId.end(); ++it) {
			const graph::Edge<double>& edge = graph.GetEdge(*it);
			serialize::Edge* ser_edge = ser_edges_list.add_edges();			
			ser_edge->set_from(static_cast<uint32_t>(edge.from));
			ser_edge->set_to(static_cast<uint32_t>(edge.to));

			//Имя остановки отправления в ребре меняем на её id, для того что бы имена остановок не дублировались
			size_t from_name_id = catalog_.StopFind(edge.fromName)->id;
			ser_edge->set_from_name_id(static_cast<uint32_t>(from_name_id));

			//Имя остановки прибытия в ребре меняем на её id, для того что бы имена остановок не дублировались
			size_t to_name_id = catalog_.StopFind(edge.toName)->id;
			ser_edge->set_to_name_id(static_cast<uint32_t>(to_name_id));

			//Если тип ребра не ожидание а маршрут, то название равно названию маршрута, в противном случае -  названию остановки
			size_t route_name_id = from_name_id;
			if (edge.type == "Bus"){
				route_name_id = catalog_.BusFind(edge.name)->id;
			}
			ser_edge->set_route_name_id(static_cast<uint32_t>(route_name_id));

			ser_edge->set_route_type(edge.type);
			ser_edge->set_weight(edge.weight);
			ser_edge->set_stop_count(edge.stopCount);
		}
	}	
	ser_graph.set_vertex_count(static_cast<uint32_t>(graphSize));
	*ser_graph.mutable_edges() = std::move(ser_edges_list);
	return ser_graph;
}

void Serialization::DeserializeRouter(serialize::TransportRouter srouter) {
	std::unordered_map<std::string, double> settings;

	for (const auto& [key, value] : *srouter.mutable_settings()) {
		settings[key] = value;
	}

	route_.SetSettings(std::move(settings));
	route_.SetGraph(DeserializeGraph(std::move(*srouter.mutable_graph())));
}

graph::DirectedWeightedGraph<double> Serialization::DeserializeGraph(serialize::Graph&& graph) {
	graph::DirectedWeightedGraph<double> result_graph(graph.vertex_count());
	for (const auto& ser_edge : *graph.mutable_edges()->mutable_edges()) {
		graph::Edge<double> edge;
		edge.from = ser_edge.from();
		edge.to = ser_edge.to();
		edge.fromName = stopIt[ser_edge.from_name_id()]->name;
		edge.toName = stopIt[ser_edge.to_name_id()]->name;
		edge.type = ser_edge.route_type();

		if (edge.type == "Bus") {
			edge.name = busIt[ser_edge.route_name_id()]->name;
		}else {
			edge.name = stopIt[ser_edge.route_name_id()]->name;
		}

		edge.weight = ser_edge.weight();
		edge.stopCount = ser_edge.stop_count();

		result_graph.AddEdge(edge);
	}

	return result_graph;
}

void Serialization::AddUnderLayerColor(serialize::MapRenderer& smap, const svg::Color& textColor) {
	if (std::holds_alternative<std::string>(textColor)) {
		serialize::ColorString clr_str;
		clr_str.set_underlayer_color(std::get<std::string>(textColor));
		*smap.mutable_underlayer_color_string() = std::move(clr_str);
	}

	if (std::holds_alternative<svg::Rgb>(textColor)) {
		serialize::ColorArray clr_ar;
		clr_ar.add_underlayer_color(std::get<svg::Rgb>(textColor).red);
		clr_ar.add_underlayer_color(std::get<svg::Rgb>(textColor).green);
		clr_ar.add_underlayer_color(std::get<svg::Rgb>(textColor).blue);
		*smap.mutable_underlayer_color_array() = std::move(clr_ar);
	}

	if (std::holds_alternative<svg::Rgba>(textColor)) {
		serialize::ColorArray clr_ar;
		clr_ar.add_underlayer_color(static_cast<double>(std::get<svg::Rgba>(textColor).red));
		clr_ar.add_underlayer_color(static_cast<double>(std::get<svg::Rgba>(textColor).green));
		clr_ar.add_underlayer_color(static_cast<double>(std::get<svg::Rgba>(textColor).blue));
		clr_ar.add_underlayer_color(std::get<svg::Rgba>(textColor).opacity);
		*smap.mutable_underlayer_color_array() = std::move(clr_ar);
	}
}

void Serialization::AddColorPallete(serialize::MapRenderer& smap, const std::vector<svg::Color>& colorPalette) {
	for (const svg::Color& itemColor : colorPalette) {
		serialize::ColorPallete plt;
		if (std::holds_alternative<std::string>(itemColor)) {
			serialize::ColorString clr_str;
			clr_str.set_underlayer_color(std::get<std::string>(itemColor));
			*plt.mutable_color_string() = std::move(clr_str);
		}

		if (std::holds_alternative<svg::Rgb>(itemColor)) {
			serialize::ColorArray clr_ar;
			clr_ar.add_underlayer_color(static_cast<double>(std::get<svg::Rgb>(itemColor).red));
			clr_ar.add_underlayer_color(static_cast<double>(std::get<svg::Rgb>(itemColor).green));
			clr_ar.add_underlayer_color(static_cast<double>(std::get<svg::Rgb>(itemColor).blue));
			*plt.mutable_color_array() = std::move(clr_ar);
		}

		if (std::holds_alternative<svg::Rgba>(itemColor)) {
			serialize::ColorArray clr_ar;
			clr_ar.add_underlayer_color(static_cast<double>(std::get<svg::Rgba>(itemColor).red));
			clr_ar.add_underlayer_color(static_cast<double>(std::get<svg::Rgba>(itemColor).green));
			clr_ar.add_underlayer_color(static_cast<double>(std::get<svg::Rgba>(itemColor).blue));
			clr_ar.add_underlayer_color(std::get<svg::Rgba>(itemColor).opacity);
			*plt.mutable_color_array() = std::move(clr_ar);
		}

		*smap.add_color_palette() = std::move(plt);
	}
}

void Serialization::DeserializeMap(serialize::MapRenderer smap) {
	std::unordered_map<std::string, transport::domain::SettingType> settings;
	settings["width"] = smap.width();
	settings["height"] = smap.height();
	settings["padding"] = smap.padding();
	settings["line_width"] = smap.line_width();
	settings["stop_radius"] = smap.stop_radius();
	settings["bus_label_font_size"] = static_cast<double>(smap.bus_label_font_size());
	settings["stop_label_font_size"] = static_cast<double>(smap.stop_label_font_size());
	settings["underlayer_width"] = smap.underlayer_width();

	serialize::BusLableOffset b_lb = smap.bus_label_offset();
	settings["bus_label_offset"] = std::pair<double, double>{b_lb.dx(), b_lb.dy()};

	serialize::StopLabelOffset s_lb = smap.stop_label_offset();
	settings["stop_label_offset"] = std::pair<double, double>{ s_lb.dx(), s_lb.dy() };

	if (smap.has_underlayer_color_string()) {
		settings["underlayer_color"] = svg::Color(smap.underlayer_color_string().underlayer_color());
	}

	if (smap.has_underlayer_color_array()) {
		serialize::ColorArray u_color = smap.underlayer_color_array();
		if (u_color.underlayer_color_size() == 3) {
			settings["underlayer_color"] = svg::Rgb(static_cast<uint8_t>(u_color.underlayer_color(0)),
													static_cast<uint8_t>(u_color.underlayer_color(1)),
													static_cast<uint8_t>(u_color.underlayer_color(2)));
		}else {
			settings["underlayer_color"] = svg::Rgba(static_cast<uint8_t>(u_color.underlayer_color(0)),
													static_cast<uint8_t>(u_color.underlayer_color(1)),
													static_cast<uint8_t>(u_color.underlayer_color(2)),
													u_color.underlayer_color(3));
		}
	}

	
	std::vector<svg::Color> colorPalette;
	for (auto it = smap.color_palette().begin(); it != smap.color_palette().end(); ++it) {
		if (it->has_color_string()) {
			colorPalette.push_back({ it->color_string().underlayer_color() });
		}

		if (it->has_color_array()) {
			serialize::ColorArray u_color = it->color_array();
			if (u_color.underlayer_color_size() == 3) {
				colorPalette.push_back(svg::Rgb(static_cast<uint8_t>(u_color.underlayer_color(0)),
													static_cast<uint8_t>(u_color.underlayer_color(1)),
													static_cast<uint8_t>(u_color.underlayer_color(2))));
			}else {
				colorPalette.push_back(svg::Rgba(static_cast<uint8_t>(u_color.underlayer_color(0)),
													static_cast<uint8_t>(u_color.underlayer_color(1)),
													static_cast<uint8_t>(u_color.underlayer_color(2)),
													u_color.underlayer_color(3)));
			}
		}
	}

	settings["color_palette"] = std::move(colorPalette);
	map_.SetSettings(settings);
}