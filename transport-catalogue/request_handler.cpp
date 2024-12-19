#include "request_handler.h"

void RequestHandler::Load(std::istream& in) {
	json_reader_.Load(in);
}

void RequestHandler::UploadAnswers(std::ostream& out){
	json_reader_.GetAnswers(out);
}

const std::optional<BusInfo> RequestHandler::GetBusStat(const std::string_view bus_name) const {
	if (!catalogue_.FindsBus(bus_name)) {
		return std::nullopt;
	}
	return catalogue_.GetBusInfo(bus_name);
}

const std::optional<std::vector<Bus*>> RequestHandler::GetBusesByStop(const std::string_view stop_name) const {
	if (!catalogue_.FindStop(stop_name)) {
		return std::nullopt;
	}
	return catalogue_.GetListOfBusStops(stop_name);
}

void RequestHandler::RenderMap(std::ostream& out) {
	using namespace std::literals;

	renderer::Settings render_settings(json_reader_.GetRenderSettings());
	renderer::MapRenderer map_renderer(render_settings);
	map_renderer.DrawBuses(std::move(catalogue_.GetListAllBuses()), out);
}