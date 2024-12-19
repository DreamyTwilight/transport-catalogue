#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace transportcatalogue {

	struct Stop {
		std::string stop_name;
		detail::Coordinates coordinates;
	};

	struct Bus {
		std::string bus_name;
		std::vector<Stop*> bus_stops;
		bool is_roundtrip = true;
	};

	struct BusInfo {
		BusInfo() = default;
		BusInfo(size_t stops_on_route, size_t unique_stops, double route_length, int route_length_in_meters);

		size_t stops_on_route = 0;
		size_t unique_stops = 0;
		double route_length = .0;
		double route_length_in_meters = 0;
		double curvature = route_length == .0 ? .0 : route_length_in_meters / route_length;
	};

	struct PairStops {
		PairStops(Stop* first, Stop* second) {
			pair_stops.first = first;
			pair_stops.second = second;
		}

		bool operator==(const PairStops& other) const {
			return other.pair_stops.first == pair_stops.first && other.pair_stops.second == pair_stops.second;
		}
		std::pair<Stop*, Stop*> pair_stops;
	};

} // transportcatalogue::
