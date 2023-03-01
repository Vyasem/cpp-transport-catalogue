#pragma once

#include <iostream>
#include <filesystem>
#include <vector>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"
#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <graph.pb.h>
#include <transport_router.pb.h>


class Serialization {
public:
	Serialization(transport::catalog::TransportCatalogue& catalog, transport::render::MapRenderer& map, transport::route::Router& route);
	void Serialize(const std::filesystem::path& path);
	void Deserialize(const std::filesystem::path& path);	

private:
	serialize::TransportCatalogue SerializeCatalog();
	void DeserializeCatalog(serialize::TransportCatalogue scatalog);

	serialize::MapRenderer SerializeMap();
	void DeserializeMap(serialize::MapRenderer smap);	

	serialize::TransportRouter SerializeRouter();
	serialize::Graph SerializeGraph(const graph::DirectedWeightedGraph<double>& graph);
	void DeserializeRouter(serialize::TransportRouter srouter);
	graph::DirectedWeightedGraph<double> DeserializeGraph(serialize::Graph&& graph);	

	void AddUnderLayerColor(serialize::MapRenderer& smap, const svg::Color& textColor);
	void AddColorPallete(serialize::MapRenderer& smap, const std::vector<svg::Color>& colorPalette);

	transport::catalog::TransportCatalogue& catalog_;
	transport::render::MapRenderer& map_;
	transport::route::Router& route_;

	std::unordered_map<int, const transport::domain::Stop*> stopIt;
	std::unordered_map<int, const transport::domain::Bus*> busIt;
};
