#include "domain.h"

namespace transportcatalogue {

	BusInfo::BusInfo(size_t stops_on_route, size_t unique_stops, double route_length, int route_length_in_meters)
		: stops_on_route(stops_on_route)
		, unique_stops(unique_stops)
		, route_length(route_length)
		, route_length_in_meters(route_length_in_meters) {
	}

} // transportcatalogue::
