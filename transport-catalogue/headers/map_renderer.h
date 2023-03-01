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
    inline const double EPSILON = 1e-6;
    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!(std::abs((max_lon - min_lon_)) < EPSILON)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!(std::abs((max_lat_ - min_lat)) < EPSILON)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

	class MapRenderer {	
	private:
		std::unordered_map<std::string, domain::SettingType> settings_;
		const std::set<const domain::Stop*, domain::StopCompare> GetAllStops(const std::deque<const domain::Bus*>& routes, std::unordered_set<geo::Coordinates, geo::CoordinateshHasher>& allCoord);
		std::pair<svg::Text, svg::Text> DrawText(const std::string name, svg::Point point, svg::Color color, bool isBus = true);
		svg::Circle DrawCircle(svg::Point point);
	public:
		MapRenderer() = default;
		void SetSettings(std::unordered_map<std::string, domain::SettingType> settings);
		const std::unordered_map<std::string, domain::SettingType>& GetSettings();
		void Draw(std::ostream& out, std::deque<const domain::Bus*> routes);		
	};
}
}