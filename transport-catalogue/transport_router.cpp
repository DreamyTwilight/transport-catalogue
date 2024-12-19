#include "transport_router.h"

namespace router {
	CreateGraphAndRoute::CreateGraphAndRoute(transportcatalogue::TransportCatalogue& catalogue, RoutingSettings routing_settings)
		:catalogue_(catalogue),
		routing_settings_(routing_settings),
		graph_(catalogue.GetCountStops() * 2),  // пустой граф
		all_stops_ptr_(catalogue.GetListPtrAllStops()) {
		CreateEdgeFromAndToByStops(all_stops_ptr_); //to - четные, from - нечетные
		std::vector<Bus*> list_ptr_buses = catalogue_.GetListAllBuses();
		for (Bus* ptr_bus : list_ptr_buses) {
			if (ptr_bus->bus_stops.empty() || ptr_bus->bus_stops.size() == 1) { continue; }
			MakeEdgeBus(ptr_bus);
		}
		router_u_ptr_ = std::make_unique<graph::Router<double>>(graph_);
	}

	std::optional<std::pair<std::vector<IdEgeInfoForPrint>, double>> CreateGraphAndRoute::BuildRoute(const Stop* from, const Stop* to) const {
		auto route_info = router_u_ptr_->BuildRoute(ptr_stop_id_edge_to_.at(catalogue_.FindStop(from->stop_name)), 
													ptr_stop_id_edge_to_.at(catalogue_.FindStop(to->stop_name)));
		if (!route_info.has_value()) { 
			return {};
		}
		std::vector<IdEgeInfoForPrint> vector_info_edges;
		for (graph::EdgeId edges_info : route_info.value().edges) {
			vector_info_edges.push_back(GetEdgeInfoForPrint(edges_info));
		}
		return std::optional<std::pair<std::vector<IdEgeInfoForPrint>, double>> {std::pair{ vector_info_edges, route_info.value().weight }};
	}

	IdEgeInfoForPrint CreateGraphAndRoute::GetEdgeInfoForPrint(const graph::EdgeId id) const {
		return id_edge_dop_info_.at(id);
	}

	double CreateGraphAndRoute::CalculateWeight(const int distance) {
		return distance * 1.0 / (routing_settings_.bus_velocity * 1000.0 / 60);
	}

	void CreateGraphAndRoute::CreateEdgeFromAndToByStops(std::vector<Stop*> all_stops_ptr_) {
		graph::VertexId id = 0;
		for (const auto& ptr_stop : all_stops_ptr_) {  // пары to-from ожидания
			ptr_stop_id_edge_to_[ptr_stop] = id;

			id_edge_dop_info_[id] = { nullptr, ptr_stop, 0, routing_settings_.bus_wait_time * 1.0 };
			id_edge_ptr_stop_[id] = ptr_stop;
			graph_.AddEdge({ id, ++id, routing_settings_.bus_wait_time * 1.0 });

			id_edge_dop_info_[id].ptr_stop = ptr_stop;
			id_edge_ptr_stop_[id] = ptr_stop;
			graph_.AddEdge({ id, id, 0.0 });
			++id;
		}
	}

	void CreateGraphAndRoute::MakeEdgeBus(const Bus* ptr_bus) {

		for (auto it_curent_stop = ptr_bus->bus_stops.begin(); it_curent_stop < ptr_bus->bus_stops.end(); ++it_curent_stop) {
			if (it_curent_stop != ptr_bus->bus_stops.end()) {
				auto it_first_stop = it_curent_stop;
				auto it_second_stop = it_curent_stop + 1;
				int distance = 0;
				int span = 0;
				while (it_second_stop < ptr_bus->bus_stops.end()) {
					distance += catalogue_.GetDistanceBetweenStops((*it_first_stop)->stop_name, (*it_second_stop)->stop_name);
					graph_.AddEdge({ ptr_stop_id_edge_to_[*it_curent_stop] + 1, ptr_stop_id_edge_to_[*it_second_stop]
						, CalculateWeight(distance) });
					id_edge_dop_info_[graph_.GetEdgeCount() - 1] = { ptr_bus, nullptr, ++span, CalculateWeight(distance) };
					++it_first_stop;
					++it_second_stop;
				}
			}
			if (!ptr_bus->is_roundtrip) {
				if (it_curent_stop == ptr_bus->bus_stops.begin()) { continue; }
				auto it_first_stop = it_curent_stop;
				auto it_second_stop = it_curent_stop - 1;
				int distance = 0;
				int span = 0;
				while (it_second_stop > ptr_bus->bus_stops.begin()) {
					distance += catalogue_.GetDistanceBetweenStops((*it_first_stop)->stop_name, (*it_second_stop)->stop_name);
					graph_.AddEdge({ ptr_stop_id_edge_to_[*it_curent_stop] + 1, ptr_stop_id_edge_to_[*it_second_stop]
						, CalculateWeight(distance) });
					id_edge_dop_info_[graph_.GetEdgeCount() - 1] = { ptr_bus, nullptr, ++span, CalculateWeight(distance) };
					--it_first_stop;
					--it_second_stop;
				}
				distance += catalogue_.GetDistanceBetweenStops((*it_first_stop)->stop_name, (*it_second_stop)->stop_name);
				graph_.AddEdge({ ptr_stop_id_edge_to_[*it_curent_stop] + 1, ptr_stop_id_edge_to_[*it_second_stop]
					, CalculateWeight(distance) });
				id_edge_dop_info_[graph_.GetEdgeCount() - 1] = { ptr_bus, nullptr, ++span, CalculateWeight(distance) };
			}
		}
	}
	
}// router::