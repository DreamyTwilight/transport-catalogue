#include "svg.h"
#include <iomanip>
#include <sstream>
#include <regex>

namespace svg {

    using namespace std::literals;

    Rgb::Rgb(int red, int green, int blue)
        : red(red)
        , green(green)
        , blue(blue) {}

    Rgba::Rgba(int red, int green, int blue, double opacity)
        : red(red)
        , green(green)
        , blue(blue)
        , opacity(opacity) {}

    void PrintColorStrHTML::operator () (std::monostate) const {
        out << "none"sv;
    }
    void PrintColorStrHTML::operator () (std::string word) const {
        out << word;
    }
    void PrintColorStrHTML::operator () (Rgb rgb) const {
        out << "rgb("sv << static_cast<int>(rgb.red) << ","sv 
            << static_cast<int>(rgb.green) << ","sv 
            << static_cast<int>(rgb.blue) << ")"sv;
    }
    void PrintColorStrHTML::operator () (Rgba rgba) const {
        out << "rgba("sv << static_cast<int>(rgba.red) << ","sv 
            << static_cast<int>(rgba.green) << ","sv 
            << static_cast<int>(rgba.blue) << "," << rgba.opacity << ")"sv;
    }

    std::ostream& operator<<(std::ostream& out, Color color) {
        std::ostringstream strm;
        visit(PrintColorStrHTML{ strm }, color);
        return  out << strm.str();
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        RenderContext ctx(out, 2, 2);
        for (size_t i = 0; i < objects_.size(); ++i) {
            objects_[i]->Render(ctx);
        }

        out << "</svg>"sv;
    }

    std::ostream& operator << (std::ostream& os, const StrokeLineCap& line_cup)
    {
        switch (line_cup) {
        case StrokeLineCap::BUTT:
            return os << "butt"sv;
        case StrokeLineCap::ROUND:
            return os << "round"sv;
        case StrokeLineCap::SQUARE:
            return os << "square"sv;
        default:
            return os << ""sv;
        }
    }

    std::ostream& operator << (std::ostream& os, const StrokeLineJoin& line_join)
    {
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            return os << "arcs"sv;
        case StrokeLineJoin::BEVEL:
            return os << "bevel"sv;
        case StrokeLineJoin::MITER:
            return os << "miter"sv;
        case StrokeLineJoin::MITER_CLIP:
            return os << "miter-clip"sv;
        case StrokeLineJoin::ROUND:
            return os << "round"sv;
        default:
            return os << ""sv;
        }
    }

    // ---------- Circle ------------------
    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x
            << "\" cy=\""sv << center_.y << "\" "sv
            << "r=\""sv << radius_ << "\""sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(out);
        out << "/>"sv;
    }
    // ---------- Polyline ------------------
    std::string DoubleToString(double num) {
        std::stringstream ss;
        ss << num;
        std::string str;
        ss >> str;
        return str;
    }

    Polyline& Polyline::AddPoint(Point point) {
        if (!points_.empty()) {
            points_ += " "s;
        }
        points_ += DoubleToString(point.x) + ","s + DoubleToString(point.y);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline"sv;
        out << " points=\""sv << points_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    std::string Text::ConvertText(const std::string data) const {
        size_t pos = 0;
        std::string result = data;
        for (auto tmp : symbols_for_replace_) {
            pos = result.find(tmp.first);
            while (pos != std::string::npos) {
                result.replace(pos, 1, tmp.second);
                pos = result.find(tmp.first, ++pos);
            }
        }
        return result;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << pos_.x
            << "\" y=\""sv << pos_.y
            << "\" dx=\""sv << offset_.x
            << "\" dy=\""sv << offset_.y
            << "\" font-size=\""sv << size_;
        if (font_family_ != ""s) {
            out << "\" font-family=\""sv << font_family_;
        }
        if (font_weight_ != ""s) {
            out << "\" font-weight=\""sv << font_weight_;
        }

        out << "\">"sv << ConvertText(data_) << "</text>"sv;
    }
}  // namespace svg