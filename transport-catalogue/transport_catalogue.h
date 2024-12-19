#pragma once

#include <deque>
#include <string>
#include <string_view>	
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <algorithm>
#include <memory>
#include <cassert>

#include "geo.h"
#include "domain.h"


namespace transportcatalogue {

	struct PairStopsHasher {
		size_t operator()(const PairStops pair_stops) const {
			return size_t(pair_stops.pair_stops.first) + size_t(pair_stops.pair_stops.second) * 7;
		}
	};

	class TransportCatalogue {
	public:
		void AddStop(const std::string& stop, const detail::Coordinates& coordinates);
		std::vector<Bus*> GetListOfBusStops(std::string_view stop_name) const;
		Stop* FindStop(std::string_view stop) const;
		std::vector<Stop*> GetListPtrAllStops() const;
		std::size_t GetCountStops() const;

		void AddBus(const std::string& bus, const std::vector<std::string_view>& stops, const bool is_roundtrip);
		BusInfo GetBusInfo(std::string_view bus) const;
		Bus* FindsBus(std::string_view bus) const;
		std::vector<Bus*> GetListAllBuses() const;

		void SetDistanceBetweenStops(const std::string_view stop_a, const std::string_view stop_b, int distance);
		int GetDistanceBetweenStops(const std::string_view stop_a, const std::string_view stop_b);

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stop_name_to_ptr_;
		std::unordered_map<PairStops, int, PairStopsHasher> distance_between_stops_;

		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> buses_name_to_ptr_;

		std::size_t GetCountStopsBus(std::string_view bus) const;
		std::size_t GetCountUniqueStopsBus(std::string_view bus) const;
		double GetBusRouteLength(std::string_view bus) const;

		size_t CalculationUniqueStops(std::string_view bus) const;
		double CalculationRouteLengthGeographical(std::string_view bus) const;
		int CalculationRouteLengthInMeters(std::string_view bus) const;
	};

} // end transportcatalogue::
