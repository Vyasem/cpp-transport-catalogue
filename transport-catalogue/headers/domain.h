#pragma once
#include "geo.h"
#include "svg.h"
#include <string_view>
#include <string>
#include <deque>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <cmath>

namespace transport {
namespace domain {
    using SettingType = std::variant<double, std::pair<double, double>, svg::Color, std::vector<svg::Color>>;

    struct Stop {
        std::string name = "";
        geo::Coordinates coord = { 0.0, 0.0 };
        Stop() = default;
        Stop(std::string_view name_, geo::Coordinates coord_) :name(name_), coord(coord_) {};
        bool operator==(const Stop& other) const {
            return name == other.name;
        }
        bool operator!=(const Stop& other) const {
            return !(*this == other);
        }            
    };

    struct StopCompare{
        bool operator()(const Stop* lhs, const Stop* rhs) const {
            return lhs->name < rhs->name;
        }
    };

    struct Bus {
        std::string name = "";
        std::deque<const Stop*> stops = {};
        bool loope = false;
        Bus() = default;
        Bus(std::string_view name_, std::deque<const Stop*> stops_, bool loope_) :name(name_), stops(std::move(stops_)), loope(loope_) {};
    };

    struct Route {
        std::string_view name = "";
        size_t stops = 0;
        size_t uStops = 0;
        double length = 0.0;
        double curvature = 0.0;
        Route() = default;
        Route(std::string_view name_, size_t stops_, size_t uStops_, double length_, double curvature_) :name(name_), stops(stops_), uStops(uStops_), length(length_), curvature(curvature_) {};
    };

	struct StopLengthHasher {
		size_t operator()(std::pair<Stop*, Stop*> key) const {
			return static_cast<size_t>(key.first->name.size() * key.first->coord.lat * key.first->coord.lng * key.second->name.size() * key.second->coord.lat * key.second->coord.lng);
		}
	};    

    struct DistanceBwStops {
        std::string_view fromStop_;
        std::string_view toStop_;
        int distance_ = 0;
        DistanceBwStops() = default;
        DistanceBwStops(std::string_view from, std::string_view to, int distance) :fromStop_(from), toStop_(to), distance_(distance) {};
    };
}
}