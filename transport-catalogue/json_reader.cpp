#include "json_reader.h"

#include <map>

using namespace std::literals;

void JSONReader::AddStopToCash(const json::Node& node) {
	using namespace std::literals;

	StopJSON stop_json;

	for (const auto& [key, value] : node.AsMap()) {
		if (key == "name"s) {
			stop_json.name = value.AsString();
		}
	}

	for (const auto& [key, value] : node.AsMap()) {
		if (key == "latitude"s) {
			stop_json.latitude = value.AsDouble();
		}
		else if (key == "longitude"s) {
			stop_json.longitude = value.AsDouble();
		}
		else if (key == "road_distances"s) {
			for (const auto& [stop_name, distance] : value.AsMap()) {
				stop_json.distance_to_stop.push_back({ stop_name,distance.AsInt() });
			}
		}
	}
	stops_cache_.push_back(std::move(stop_json));
}

void JSONReader::AddBusToCash(const json::Node& node) {
	using namespace std::literals;

	BusJSON buses_json;

	for (const auto& [key, value] : node.AsMap()) {
		if (key == "name"s) {
			buses_json.name = value.AsString();
		}
		else if (key == "is_roundtrip"s) {
			buses_json.is_roundtrip = value.AsBool();
		}
		else if (key == "stops"s) {
			for (const auto& stop_name : value.AsArray()) {
				buses_json.bus_stops.push_back(stop_name.AsString());
			}
		}
	}
	bus_cache_.push_back(std::move(buses_json));
}

void JSONReader::ReadNode(const json::Node& node) {
	using namespace std::literals;
	for (const auto& cur_value : node.AsArray()) {
		for (const auto& [key, value] : cur_value.AsMap()) {
			if (key == "type"s) {
				if (value.AsString() == "Stop"s) {
					AddStopToCash(cur_value);
				}
				else if (value.AsString() == "Bus"s) {
					AddBusToCash(cur_value);
				}
			}
		}
	}
}

void JSONReader::WriteCacheToCatalogue() {

	for (const auto& stop : stops_cache_) {
		catalogue_.AddStop(stop.name, { stop.latitude, stop.longitude });
	}

	for (const auto& stop_lt : stops_cache_) {
		for (const auto& [stop_rt, distance] : stop_lt.distance_to_stop) {
			catalogue_.SetDistanceBetweenStops(stop_lt.name, stop_rt, distance);
		}
	}

	for (const auto& bus : bus_cache_) {
		catalogue_.AddBus(bus.name, bus.bus_stops, bus.is_roundtrip);
	}
}

void JSONReader::ReadRequestNode(const json::Node& node) {
	for (const auto& cur_value : node.AsArray()) {
		Request request;
		for (const auto& [key, value] : cur_value.AsMap()) {
			if (key == "id"s) {
				request.id = value.AsInt();
			}
			else if (key == "type"s) {
				request.type = value.AsString();
			}
			else if (key == "name"s) {
				request.name = value.AsString();
			}
			else if (key == "from"s) {
				request.from = value.AsString();
			}
			else if (key == "to"s) {
				request.to = value.AsString();
			}
		}
		requests_.push_back(request);
	}
}

svg::Color ConvertJsonArrToSvgColor(const json::Node& node) {
	svg::Color color;
	if (node.IsString()) {
		color = node.AsString();
	}
	else if (node.AsArray().size() == 3) {
		color = svg::Rgb(node.AsArray()[0].AsInt(),
			node.AsArray()[1].AsInt(), node.AsArray()[2].AsInt());
	}
	else if (node.AsArray().size() == 4) {
		color = svg::Rgba( node.AsArray()[0].AsInt(),
			node.AsArray()[1].AsInt(), node.AsArray()[2].AsInt(),
			node.AsArray()[3].AsDouble());
	}
	return color;
}

void JSONReader::ReadRenderNode(const json::Node& node) {
	for (const auto& [key, value] : node.AsMap()) {
		if (key == "width"s) {
			settings_.width = value.AsDouble();
		}
		else if (key == "height"s) {
			settings_.height = value.AsDouble();
		}
		else if (key == "padding"s) {
			settings_.padding = value.AsDouble();
		}
		else if (key == "stop_radius"s) {
			settings_.stop_radius = value.AsDouble();
		}
		else if (key == "line_width"s) {
			settings_.line_width = value.AsDouble();
		}
		else if (key == "bus_label_font_size"s) {
			settings_.bus_label_font_size = value.AsInt();
		}
		else if (key == "stop_label_font_size"s) {
			settings_.stop_label_font_size = value.AsInt();
		}
		else if (key == "underlayer_width"s) {
			settings_.underlayer_width = value.AsDouble();
		}
		else if (key == "bus_label_offset"s) {
			settings_.bus_label_offset = svg::Point(value.AsArray()[0].AsDouble(),
				value.AsArray()[1].AsDouble());
		}
		else if (key == "stop_label_offset"s) {
			settings_.stop_label_offset = svg::Point(value.AsArray()[0].AsDouble(),
				value.AsArray()[1].AsDouble());
		}
		else if (key == "underlayer_color"s) {
			settings_.underlayer_color = std::move(ConvertJsonArrToSvgColor(value));
		}
		else if (key == "color_palette"s) {
			for (const auto& color_value : value.AsArray()) {
				settings_.color_palette.push_back(std::move(ConvertJsonArrToSvgColor(color_value)));
			}
		}
	}
}

void JSONReader::ReadRoutingSettingsNode(const json::Node& node) {
	for (const auto& [key, value] : node.AsMap()) {
		if (key == "bus_wait_time"s) {
			routing_settings_.bus_wait_time = value.AsInt();
		}
		else if (key == "bus_velocity"s) {
			routing_settings_.bus_velocity = value.AsInt();
		}
	}
}

renderer::Settings JSONReader::GetRenderSettings() {
	return settings_;
}

router::RoutingSettings JSONReader::GetRoutingSettings() {
	return routing_settings_;
}

void JSONReader::GetAnswers(std::ostream& output) {
	json::Array arr_answers;
	arr_answers.reserve(requests_.size());
	for (const auto& [id, type, name, from, to] : requests_) {
		if (type == "Stop"s) {
			if (!catalogue_.FindStop(name)) {
				arr_answers.push_back(json::Dict({ 
					{"error_message", json::Node("not found")},
					{"request_id" , json::Node(id)}
					}));
			}
			else {
				auto vector_buses_ptr = catalogue_.GetListOfBusStops(name);
				json::Array buses;
				buses.reserve(vector_buses_ptr.size());
				for (const auto bus_name:vector_buses_ptr) {
					buses.push_back(json::Node(bus_name->bus_name));
				}
				arr_answers.emplace_back(json::Dict({
					{"buses", json::Node(buses)},
					{"request_id" , json::Node(id)}
					}));
			}
		}
		else if (type == "Bus"s) {
			if (!catalogue_.FindsBus(name)) {
				arr_answers.emplace_back(json::Dict({
					{"error_message", json::Node("not found")},
					{"request_id" , json::Node(id)}
					}));
			}
			else {
				const auto bus_stat = catalogue_.GetBusInfo(name);
				arr_answers.emplace_back(json::Dict({
					{"curvature", json::Node(bus_stat.curvature)},
					{"request_id", json::Node(id)},
					{"route_length", json::Node(bus_stat.route_length_in_meters)},
					{"stop_count", json::Node(int(bus_stat.stops_on_route))},
					{"unique_stop_count", json::Node(int(bus_stat.unique_stops))}
					}));
			}
		}
		else if (type == "Route"s) {  
			static router::CreateGraphAndRoute grapher_route(catalogue_, routing_settings_);
			std::optional<std::pair<std::vector<router::IdEgeInfoForPrint>, double>> route_info = 
				grapher_route.BuildRoute(catalogue_.FindStop(from), catalogue_.FindStop(to));
			
			if (!route_info.has_value()) {
				arr_answers.emplace_back(json::Dict({
					{"error_message", json::Node("not found")},
					{"request_id" , json::Node(id)}
					}));
			}
			else {
				json::Array items_route;
				for (const router::IdEgeInfoForPrint edges_info : route_info.value().first) {
					if (edges_info.IsBus()) {
						items_route.emplace_back(json::Dict{
							{"type", json::Node("Bus")},
							{"bus",  json::Node(edges_info.ptr_bus_route->bus_name)},
							{"span_count", json::Node(edges_info.span)},
							{"time", json::Node(edges_info.weight)}
							});
					}
					else {
						items_route.emplace_back(json::Dict{
							{"stop_name",  json::Node(edges_info.ptr_stop->stop_name)},
							{"time", json::Node(edges_info.weight)},
							{"type", json::Node("Wait")}
							});
					}
				}
				arr_answers.emplace_back(json::Dict({
						{"request_id" , json::Node(id)},
						{"total_time" , json::Node(route_info.value().second)},
						{"items" , items_route}
					}));
			}
		}
		else if (type == "Map"s) {
			std::stringstream ss_draw_buses;
			renderer::MapRenderer map_renderer(settings_);
			map_renderer.DrawBuses(std::move(catalogue_.GetListAllBuses()), ss_draw_buses);
			arr_answers.emplace_back(json::Dict({
				{"map", json::Node(ss_draw_buses.str())},
				{"request_id" , json::Node(id)}
				}));
		}
	}
	json::Print(json::Document(json::Builder(json::Node(arr_answers)).Build()), output);
}

void JSONReader::Load(std::istream& input) {

	json::Document doc = json::Document{ json::Load(input) };

	for (const auto& [key, value] : doc.GetRoot().AsMap()) {
		if (key == "base_requests"s) {
			ReadNode(value);
			WriteCacheToCatalogue();
		}
		else if (key == "stat_requests"s) {
			ReadRequestNode(value);
		}
		else if (key == "render_settings"s) {
			ReadRenderNode(value);
		}
		else if (key == "routing_settings"s) {
			ReadRoutingSettingsNode(value);
		}
	}
}