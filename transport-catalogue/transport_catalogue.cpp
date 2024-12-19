#include "transport_catalogue.h"

#include <iostream>


namespace transportcatalogue {

	void TransportCatalogue::AddStop(const std::string& stop, const detail::Coordinates& coordinates) {
		stops_.push_front({ std::move(stop), std::move(coordinates) });
		stop_name_to_ptr_[stops_[0].stop_name] = &stops_[0];
	}

	std::vector<Bus*> TransportCatalogue::GetListOfBusStops(std::string_view stop_name) const {
		if (!FindStop(stop_name)) {
			return  {};
		}
		std::vector<Bus*> buses_list;
		for (const auto& [bus_name, bus_stops, is_roundtrip] : buses_) {
			if (std::find(bus_stops.begin(), bus_stops.end(), stop_name_to_ptr_.at(stop_name)) != bus_stops.end()) {
				buses_list.push_back(buses_name_to_ptr_.at(bus_name));
			}
		}
		if (!buses_list.empty()) {
			std::sort(buses_list.begin(), buses_list.end(), [](Bus* a, Bus* b) {return a->bus_name < b->bus_name; });
		}
		return buses_list;
	}

	std::vector<Bus*> TransportCatalogue::GetListAllBuses() const {
		std::vector<Bus*> result;
		result.reserve(buses_name_to_ptr_.size());
		for (const auto& [bus_name, bus_ptr] : buses_name_to_ptr_) {
			result.push_back(bus_ptr);
		}
		std::sort(result.begin(), result.end(), [](Bus* l, Bus* r) {return l->bus_name < r->bus_name; });
		return result;
	}


	Stop* TransportCatalogue::FindStop(std::string_view stop) const {
		return stop_name_to_ptr_.count(stop) ? stop_name_to_ptr_.at(stop) : nullptr;
	}
	
	std::vector<Stop*> TransportCatalogue::GetListPtrAllStops() const {
		std::vector<Stop*> result;
		for (const auto& stop: stop_name_to_ptr_) {
			result.push_back(stop.second);
		}
		return result;
	}

	std::size_t TransportCatalogue::GetCountStops() const {
		return stop_name_to_ptr_.size();
	}

	// начальная остановка для любых видов маршрутов и конечная для не кольцевого, всегда считаются двойными с расстоянием 0 по умолчанию
	void TransportCatalogue::AddBus(const std::string& bus, const std::vector<std::string_view>& stops, const bool is_roundtrip) {
		using namespace std::literals;
		buses_.push_front({ std::move(bus), {} });
		buses_name_to_ptr_[buses_[0].bus_name] = &buses_[0];
		buses_[0].is_roundtrip = is_roundtrip;
		for (std::string_view stop_name : stops) {
			assert(FindStop(stop_name) != nullptr);
			Stop* ptr_stop = stop_name_to_ptr_.at(stop_name);
			buses_[0].bus_stops.push_back(ptr_stop);
		}
	}

	std::size_t TransportCatalogue::GetCountStopsBus(std::string_view bus) const {
		return buses_name_to_ptr_.at(bus)->is_roundtrip ? buses_name_to_ptr_.at(bus)->bus_stops.size() : buses_name_to_ptr_.at(bus)->bus_stops.size() * 2 - 1;
	}

	std::size_t TransportCatalogue::GetCountUniqueStopsBus(std::string_view bus) const {
		return CalculationUniqueStops(bus);
	}

	double TransportCatalogue::GetBusRouteLength(std::string_view bus) const {
		return CalculationRouteLengthGeographical(bus);
	}

	Bus* TransportCatalogue::FindsBus(std::string_view bus) const {
		return buses_name_to_ptr_.count(bus) ? buses_name_to_ptr_.at(bus) : nullptr;
	}

	BusInfo TransportCatalogue::GetBusInfo(std::string_view bus) const {
		if (!FindsBus(bus)) {
			return {};
		}
		return BusInfo{ GetCountStopsBus(bus),GetCountUniqueStopsBus(bus), GetBusRouteLength(bus) , CalculationRouteLengthInMeters(bus) };
	}

	void TransportCatalogue::SetDistanceBetweenStops(const std::string_view stop_a, const std::string_view stop_b, const int distance) {
		assert(FindStop(stop_a) != nullptr);
		assert(FindStop(stop_b) != nullptr);
		distance_between_stops_[PairStops(FindStop(stop_a), FindStop(stop_b))] = distance;
		if (distance_between_stops_.find(PairStops(FindStop(stop_b), FindStop(stop_a))) == distance_between_stops_.end()) {
			distance_between_stops_[PairStops(FindStop(stop_b), FindStop(stop_a))] = distance;
		}
		if (distance_between_stops_.find(PairStops(FindStop(stop_a), FindStop(stop_a))) == distance_between_stops_.end()) {
			distance_between_stops_[PairStops(FindStop(stop_a), FindStop(stop_a))] = 0;
		}
		if (distance_between_stops_.find(PairStops(FindStop(stop_b), FindStop(stop_b))) == distance_between_stops_.end()) {
			distance_between_stops_[PairStops(FindStop(stop_b), FindStop(stop_b))] = 0;
		}
	}

	int TransportCatalogue::GetDistanceBetweenStops(const std::string_view stop_a, const std::string_view stop_b) {
		assert(FindStop(stop_a) != nullptr);
		assert(FindStop(stop_b) != nullptr);
		return distance_between_stops_.find(PairStops(FindStop(stop_a), FindStop(stop_b))) == distance_between_stops_.end() ? 0
			: distance_between_stops_.at(PairStops(FindStop(stop_a), FindStop(stop_b)));
	}

	std::size_t TransportCatalogue::CalculationUniqueStops(std::string_view bus) const {
		std::unordered_set<Stop*> un_set;
		for (auto stop_ptr : buses_name_to_ptr_.at(bus)->bus_stops) {
			un_set.insert(stop_ptr);
		}
		return un_set.size();
	}

	double TransportCatalogue::CalculationRouteLengthGeographical(std::string_view bus) const {
		size_t curent_stop = 0;
		double route_length = 0;
		while (curent_stop < buses_name_to_ptr_.at(bus)->bus_stops.size() - 1) {
			route_length += ComputeDistance(buses_name_to_ptr_.at(bus)->bus_stops.at(curent_stop)->coordinates,
				buses_name_to_ptr_.at(bus)->bus_stops.at(curent_stop + 1)->coordinates);
			++curent_stop;
		}

		if (!buses_name_to_ptr_.at(bus)->is_roundtrip) {
			route_length *= 2;
		}

		return route_length;
	}

	int TransportCatalogue::CalculationRouteLengthInMeters(std::string_view bus) const {
		Stop* first_stop = buses_name_to_ptr_.at(bus)->bus_stops.at(0);
		Stop* second_stop = nullptr;
		
		int distance = buses_name_to_ptr_.at(bus)->is_roundtrip ? 0:
			distance_between_stops_.at(PairStops(first_stop, first_stop));
		
		for (size_t i = 1; i < buses_name_to_ptr_.at(bus)->bus_stops.size(); ++i) {
			second_stop = buses_name_to_ptr_.at(bus)->bus_stops.at(i);
			distance += distance_between_stops_.at(PairStops(first_stop, second_stop));
			first_stop = second_stop;
		}
		
		if (!buses_name_to_ptr_.at(bus)->is_roundtrip) {
			distance += distance_between_stops_.at(PairStops(second_stop, second_stop));
			for (auto i = buses_name_to_ptr_.at(bus)->bus_stops.size() - 1; i-- > 0 ;) {
				second_stop = buses_name_to_ptr_.at(bus)->bus_stops.at(i);
				distance += distance_between_stops_.at(PairStops(first_stop, second_stop));
				first_stop = second_stop;
			}
		}
		return distance;
	}
} // end transportcatalogue