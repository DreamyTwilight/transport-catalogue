#include "map_renderer.h"

#include <unordered_set>

namespace renderer {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    std::vector<transportcatalogue::Stop*> GetListUniqueStopPtr(const std::vector<transportcatalogue::Bus*>& buses_list) {
        std::unordered_set<transportcatalogue::Stop*> set_stops_bus_routes;
        
        for (const auto& bus : buses_list) {
            for (const auto& stop_ptr : bus->bus_stops) {
                set_stops_bus_routes.insert(stop_ptr);
            }
        }
        std::vector<transportcatalogue::Stop*> list_stops_ptr(set_stops_bus_routes.begin()
            , set_stops_bus_routes.end());

        std::sort(list_stops_ptr.begin(), list_stops_ptr.end(),
            [](transportcatalogue::Stop* lhr, transportcatalogue::Stop* rhr) { return lhr->stop_name < rhr->stop_name; });

        return list_stops_ptr;
    }

    SphereProjector CreateSphereProjector(std::vector<transportcatalogue::Stop*> list_stops_ptr, const Settings& settings_) {
        std::vector<transportcatalogue::detail::Coordinates> vector_coordinates;
        for (const auto& stop_ptr : list_stops_ptr) {
            vector_coordinates.push_back(stop_ptr->coordinates);
        }

        SphereProjector sphere_projector(
            vector_coordinates.begin()
            , vector_coordinates.end()
            , settings_.width
            , settings_.height
            , settings_.padding);

        return sphere_projector;
    }

    void AddToSVGDocLinesBuses(svg::Document& doc
        , const std::vector<transportcatalogue::Bus*>& buses_list
        , const SphereProjector& sphere_projector
        , const Settings& settings_) {
        
        GetColorBus color_bus(settings_.color_palette);
        for (const auto& bus : buses_list) {
            if (bus->bus_stops.empty()) {
                continue;
            }
            svg::Polyline route_line_bus;
            for (const auto& stop_ptr : bus->bus_stops) {
                route_line_bus.AddPoint(sphere_projector(stop_ptr->coordinates));
            }
            if (!bus->is_roundtrip) {
                const auto bus_stops_ptr = bus->bus_stops;
                for (auto r_itr = bus_stops_ptr.crbegin() + 1; r_itr != bus_stops_ptr.crend(); ++r_itr) {
                    route_line_bus.AddPoint(sphere_projector((*r_itr)->coordinates));
                }
            }
            route_line_bus.SetStrokeColor(color_bus());
            route_line_bus.SetFillColor("none"s);
            route_line_bus.SetStrokeWidth(settings_.line_width);
            route_line_bus.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            route_line_bus.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            doc.Add(std::move(route_line_bus));
        }
    }

    void SetCommonPropertiesNameBus(const transportcatalogue::Bus* bus
        , svg::Text& route_name_bus
        , const Settings& settings_) {
        route_name_bus.SetOffset(settings_.bus_label_offset);
        route_name_bus.SetFontSize(settings_.bus_label_font_size);
        route_name_bus.SetFontFamily("Verdana"s);
        route_name_bus.SetFontWeight("bold"s);
        route_name_bus.SetData(bus->bus_name);
    }

    void SetSubstratePropertiesName(svg::Text& route_name_bus
        , const Settings& settings_) {
        route_name_bus.SetFillColor(settings_.underlayer_color);
        route_name_bus.SetStrokeColor(settings_.underlayer_color);
        route_name_bus.SetStrokeWidth(settings_.underlayer_width);
        route_name_bus.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        route_name_bus.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    
    void AddToSVGDocNameBus(svg::Document& doc
        , const transportcatalogue::Bus* bus
        , const svg::Point& pos
        , const Settings& settings_, svg::Color color_name_bus) {
        svg::Text route_substrate_name_bus;
        route_substrate_name_bus.SetPosition(pos);
        SetCommonPropertiesNameBus(bus, route_substrate_name_bus, settings_);
        SetSubstratePropertiesName(route_substrate_name_bus, settings_);
        doc.Add(std::move(route_substrate_name_bus));

        svg::Text route_name_bus;
        route_name_bus.SetPosition(pos);
        SetCommonPropertiesNameBus(bus, route_name_bus, settings_);
        route_name_bus.SetFillColor(color_name_bus);
        doc.Add(std::move(route_name_bus));
    }

    void AddToSVGDocNamesBuses(svg::Document& doc
        , const std::vector<transportcatalogue::Bus*>& buses_list
        , const SphereProjector& sphere_projector
        , const Settings& settings_) {
        
        GetColorBus color_bus(settings_.color_palette);
        for (const auto& bus : buses_list) {
            if (bus->bus_stops.empty()) {
                continue;
            }
            svg::Color color_name_bus = color_bus();
            AddToSVGDocNameBus(doc, bus, sphere_projector(bus->bus_stops[0]->coordinates), settings_, color_name_bus);
            if (!bus->is_roundtrip) {
                if (bus->bus_stops[0]->stop_name != bus->bus_stops[bus->bus_stops.size() - 1]->stop_name) {
                    AddToSVGDocNameBus(doc, bus, sphere_projector(bus->bus_stops[bus->bus_stops.size() - 1]->coordinates), settings_, color_name_bus);
                }
            }
        }
    }

    void AddToSVGDocCirclesStops(svg::Document& doc
        , const std::vector<transportcatalogue::Stop*>& list_stops_ptr
        , const SphereProjector& sphere_projector
        , const Settings& settings_) {
        for (const auto& stop : list_stops_ptr) {
            svg::Circle stops_circles;
            stops_circles.SetCenter(sphere_projector(stop->coordinates));
            stops_circles.SetRadius(settings_.stop_radius);
            stops_circles.SetFillColor("white"s);
            doc.Add(stops_circles);
        }
    }

    void SetCommonPropertiesNameStop(transportcatalogue::Stop* stop
        , svg::Text& name_stop
        , const Settings& settings_) {
        name_stop.SetOffset(settings_.stop_label_offset);
        name_stop.SetFontSize(settings_.stop_label_font_size);
        name_stop.SetFontFamily("Verdana"s);
        name_stop.SetData(stop->stop_name);
    }

    void AddToSVGDocNamesStops(svg::Document& doc
        , const std::vector<transportcatalogue::Stop*>& list_stops_ptr
        , const SphereProjector& sphere_projector
        , const Settings& settings_) {

        for (const auto& stop : list_stops_ptr) {
            svg::Text substrate_name_stop;
            substrate_name_stop.SetPosition(sphere_projector(stop->coordinates));
            SetCommonPropertiesNameStop(stop, substrate_name_stop, settings_);
            SetSubstratePropertiesName(substrate_name_stop, settings_);
            substrate_name_stop.SetFillColor(settings_.underlayer_color);
            doc.Add(std::move(substrate_name_stop));

            svg::Text name_stop;
            name_stop.SetPosition(sphere_projector(stop->coordinates));
            SetCommonPropertiesNameStop(stop, name_stop, settings_);
            name_stop.SetFillColor("black");
            doc.Add(std::move(name_stop));
        }
    }

	void MapRenderer::DrawBuses(const std::vector<transportcatalogue::Bus*>& buses_list, std::ostream& out) {
        std::vector<transportcatalogue::Stop*> list_stops_ptr = GetListUniqueStopPtr(buses_list);
        SphereProjector sphere_projector = CreateSphereProjector(list_stops_ptr, settings_);

        svg::Document doc;
        AddToSVGDocLinesBuses(doc, buses_list, sphere_projector, settings_);
        AddToSVGDocNamesBuses(doc, buses_list, sphere_projector, settings_);
        AddToSVGDocCirclesStops(doc, list_stops_ptr, sphere_projector, settings_);
        AddToSVGDocNamesStops(doc, list_stops_ptr, sphere_projector, settings_);
        doc.Render(out);
	}

    svg::Color GetColorBus::operator()() {
        auto result = it_;
        ++it_;
        if (it_ == color_palette_.end()) {
            it_ = color_palette_.begin();
        }
        return *result;
    }

    svg::Point SphereProjector::operator()(transportcatalogue::detail::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

} // renderer::