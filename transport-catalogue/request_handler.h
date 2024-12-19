#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
using namespace transportcatalogue;

class RequestHandler {
    
public:
    RequestHandler() = default;
    RequestHandler(TransportCatalogue& catalogue) :catalogue_(catalogue), json_reader_(catalogue) { }

    void Load(std::istream& in);
    void UploadAnswers(std::ostream& out);

    const std::optional<BusInfo> GetBusStat(const std::string_view bus_name) const;
    const std::optional<std::vector<Bus*>> GetBusesByStop(const std::string_view stop_name) const;

    void RenderMap(std::ostream& out);

private:
    TransportCatalogue& catalogue_;
    JSONReader json_reader_;
};
