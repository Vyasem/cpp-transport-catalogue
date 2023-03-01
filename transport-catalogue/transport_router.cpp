#include "headers/transport_router.h"
#include "headers/graph.h"
#include "headers/router.h"
#include "headers/transport_catalogue.h"

#include <unordered_map>
#include <variant>
#include <string>
#include <vector>
#include <algorithm>
#include <string_view>
#include <memory>

namespace transport {
namespace route {
	void Router::SetSettings(std::unordered_map<std::string, double>&& settings) {
		settings_ = std::move(settings);
	}

	void Router::SetGraph(graph::DirectedWeightedGraph<double>&& graph_) {
		graph = std::move(graph_);
		routerFinder = std::make_unique<graph::Router<double>>(graph);
	}

	const std::unordered_map<std::string, double>& Router::GetSettings() {
		return settings_;
	}

	const graph::DirectedWeightedGraph<double>& Router::GetGraph() {
		return graph;
	}

	void Router::CreateWaitEdge(graph::VertexId fromId, graph::VertexId toId, std::string_view name, double weight) {
		auto edgeExist = graph.GetIncidentEdges(fromId);
		if (edgeExist.begin() == edgeExist.end()) {
			graph.AddEdge(graph::Edge<double> { fromId, toId, name, name, name, "Wait", weight });
		}
	}

	double Router::CalculateEdgeWeight(const transport::domain::Stop* from, const transport::domain::Stop* to, transport::catalog::TransportCatalogue& catalog) {
		double transformSpeed = settings_["bus_velocity"] * 1000 / 60;
		auto distance = catalog.GetDistance(from, to);
		return (distance / transformSpeed);
	}

	void Router::CreateRoutes(transport::catalog::TransportCatalogue& catalog) {
		size_t uniqueStopsCount = catalog.GetUniqueStopCount();
		graph::DirectedWeightedGraph<double> result(uniqueStopsCount * 2);
		graph = std::move(result);
		std::deque<const domain::Bus*> allRoutes = catalog.GetAllRoutes();
			
		double waitTime = settings_["bus_wait_time"];
			
		for (const domain::Bus* itemRoute : allRoutes) {
			for (auto itemStopIt = itemRoute->stops.begin(); itemStopIt != itemRoute->stops.end() - 1; ++itemStopIt) {
				double derectWeight = 0;
				double backWeight = 0;
				int stopCount = 0;
				auto nextStopIt = itemStopIt + 1;
				std::pair<const transport::domain::Stop*, const transport::domain::Stop*> stopPair{ *itemStopIt , *itemStopIt };
				graph::VertexId departureVertextId = (**itemStopIt).id;
				graph::VertexId waitVertextId = (**itemStopIt).id + uniqueStopsCount;

				CreateWaitEdge(departureVertextId, waitVertextId, (**itemStopIt).name, waitTime);

				while (nextStopIt != itemRoute->stops.end()) {
					++stopCount;
					stopPair.second = *nextStopIt;
					graph::VertexId destinationVertexId = (**nextStopIt).id;
					graph::VertexId innerWaitVertexId = (**nextStopIt).id + uniqueStopsCount;

					CreateWaitEdge(destinationVertexId, innerWaitVertexId, (**nextStopIt).name, waitTime);

					derectWeight += CalculateEdgeWeight(stopPair.first, stopPair.second, catalog);
					graph.AddEdge(graph::Edge<double> { waitVertextId, destinationVertexId, (**itemStopIt).name, (**nextStopIt).name, itemRoute->name, "Bus", derectWeight, stopCount });
						
					if (!itemRoute->loope) {							
						backWeight += CalculateEdgeWeight(stopPair.second, stopPair.first, catalog);
						graph.AddEdge(graph::Edge<double> { innerWaitVertexId, departureVertextId, (**nextStopIt).name, (**itemStopIt).name, itemRoute->name, "Bus", backWeight, stopCount });
					}						
					stopPair.first = *nextStopIt;
					++nextStopIt;
				}
			}
		}			
		routerFinder = std::make_unique<graph::Router<double>>(graph);
	}

	void Router::FindRoute(const domain::Stop* from, const domain::Stop* to) {
		std::optional<graph::Router<double>::RouteInfo> res = routerFinder->BuildRoute(from->id, to->id);			
		readyRoute.reset();
		if (res.has_value()) {
			domain::Trip result;
			std::vector<graph::EdgeId> edgeIds = res.value().edges;				
			for (const graph::EdgeId& itemEdgeId : edgeIds) {
				const graph::Edge edge = graph.GetEdge(itemEdgeId);
				result.totalTime += edge.weight;
				result.items.push_back({edge.type, edge.weight, edge.name, edge.stopCount});
			}
			readyRoute = std::move(result);
		}						
	}

	const std::optional<domain::Trip>& Router::GetReadyRoute()const {
		return readyRoute;
	}
}
}