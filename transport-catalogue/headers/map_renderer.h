#pragma once

#include "svg.h"
#include "domain.h"

#include <iostream>
#include <deque>
#include <unordered_set>
#include <variant>
#include <set>

namespace transport {
	namespace render {
		class MapRenderer {	
		private:
			std::unordered_map<std::string, domain::SettingType> settings_;
			const std::set<const domain::Stop*, domain::StopCompare> AllStops(const std::deque<const domain::Bus*>& routes, std::unordered_set<geo::Coordinates, geo::CoordinateshHasher>& allCoord);
			std::pair<svg::Text, svg::Text> DrawText(const std::string name, svg::Point point, svg::Color color, bool isBus = true);
			svg::Circle DrawCircle(svg::Point point);
		public:
			MapRenderer() = default;
			void SetSettings(std::unordered_map<std::string, domain::SettingType> settings);
			void Draw(std::ostream& out, std::deque<const domain::Bus*> routes);		
		};
	}
}