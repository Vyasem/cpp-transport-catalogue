#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <variant>
#include <cstdint>

using namespace std::string_literals;

namespace svg {
    struct Rgb {
        Rgb() = default;
        Rgb(unsigned int red_, unsigned int green_, unsigned int blue_) :red(red_), green(green_), blue(blue_) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(unsigned int red_, unsigned int green_, unsigned int blue_, double opacity_) :red(red_), green(green_), blue(blue_), opacity(opacity_) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    struct ColorToStream {
        std::ostream& out;
        void operator()(std::monostate) {
            out << "none";
        }
        void operator()(std::string color) {
            out << color;
        }
        void operator()(Rgb color) {
            out << "rgb(" << color.red + 0 << "," << color.green + 0 << "," << color.blue + 0 << ")";
        }
        void operator()(Rgba color) {
            out << "rgba(" << color.red + 0 << ',' << color.green + 0 << ',' << color.blue + 0 << ',' << color.opacity << ")";
        }
    };


    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor = "none";

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        template<typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(obj));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    class Drawable {
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& object) const = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv;
                std::visit(ColorToStream{ out }, fill_color_.value());
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                std::visit(ColorToStream{ out }, stroke_color_.value());
                out << "\""sv;
            }

            if (stroke_width_) {
                out << " stroke-width=\""sv << stroke_width_.value() << "\""sv;
            }

            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv;
                switch (*stroke_line_cap_)
                {
                case svg::StrokeLineCap::BUTT: out << "butt";   break;
                case svg::StrokeLineCap::ROUND: out << "round"; break;
                case svg::StrokeLineCap::SQUARE: out << "square";  break;
                }
                out << "\""sv;
            }

            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv;
                switch (*stroke_line_join_)
                {
                case svg::StrokeLineJoin::ARCS: out << "arcs";   break;
                case svg::StrokeLineJoin::BEVEL: out << "bevel"; break;
                case svg::StrokeLineJoin::MITER: out << "miter";  break;
                case svg::StrokeLineJoin::MITER_CLIP: out << "miter-clip";  break;
                case svg::StrokeLineJoin::ROUND: out << "round";  break;
                }
                out << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);
    private:
        void RenderObject(const RenderContext& context) const override;
        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> coordinates_;
    };

    class Text final : public Object, public PathProps<Text> {
    public:
        Text& SetPosition(Point pos);       
        Text& SetOffset(Point offset);        
        Text& SetFontSize(uint32_t size);        
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);       
        Text& SetData(std::string data);
        std::string PrepareText(std::string data);
    private:
        void RenderObject(const RenderContext& context) const override;
        Point pos_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_ = "";
    };

    class Document : public ObjectContainer {
    public:
        Document() = default;
        ~Document() = default;

        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objects_ptr_;
    };
} 

