#pragma once

#include <deque>
#include <sstream>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"

struct StopJSON {
	double latitude = .0;
	double longitude = .0;
	std::string name;
	std::vector<std::pair<std::string, int>> distance_to_stop;
};

struct BusJSON {
	std::string name;
	bool is_roundtrip = true;
	std::vector<std::string_view> bus_stops;
};

struct Request {
	int id = 0;
	std::string type;
	std::string name;
	std::string from;
	std::string to;
};

class JSONReader {
public:
	JSONReader() = default;

	JSONReader(transportcatalogue::TransportCatalogue& catalogue) : catalogue_(catalogue) {}

	void Load(std::istream& input);
	void GetAnswers(std::ostream& output);
	renderer::Settings GetRenderSettings();
	router::RoutingSettings GetRoutingSettings();

private:

	transportcatalogue::TransportCatalogue& catalogue_;
	std::deque<StopJSON> stops_cache_;
	std::deque<BusJSON> bus_cache_;
	std::deque<Request> requests_;
	renderer::Settings settings_;
	router::RoutingSettings routing_settings_;

	void AddStopToCash(const json::Node& node);
	void AddBusToCash(const json::Node& node);
	void ReadNode(const json::Node& node);
	void ReadRequestNode(const json::Node& node);
	void ReadRenderNode(const json::Node& node);
	void WriteCacheToCatalogue();
	void ReadRoutingSettingsNode(const json::Node& node);

};