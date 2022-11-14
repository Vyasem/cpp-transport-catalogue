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
	}
}