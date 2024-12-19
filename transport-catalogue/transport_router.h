#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

using namespace transportcatalogue;

namespace router {

	struct RoutingSettings {
		int bus_wait_time = 1; // минуты
		int bus_velocity = 1;  // км/час
	};

	struct IdEgeInfoForPrint {
		const Bus* ptr_bus_route = nullptr;
		Stop* ptr_stop = nullptr;
		int span = 0; // количество пролетов между остановками
		double weight = 0;

		bool IsBus() const {
			return ptr_bus_route != nullptr;
		}
	};

	class CreateGraphAndRoute {
	public:
		CreateGraphAndRoute() = default;

		explicit CreateGraphAndRoute(transportcatalogue::TransportCatalogue& catalogue, RoutingSettings routing_settings);

		std::optional<std::pair<std::vector<IdEgeInfoForPrint>, double>> BuildRoute(const Stop* from, const Stop* to) const;


	private:
		transportcatalogue::TransportCatalogue& catalogue_;
		RoutingSettings routing_settings_;
		graph::DirectedWeightedGraph<double> graph_;
		std::vector<Stop*> all_stops_ptr_;
		std::unordered_map<graph::VertexId, Stop*> id_edge_ptr_stop_;
		std::unordered_map<graph::VertexId, IdEgeInfoForPrint> id_edge_dop_info_;
		std::unordered_map<Stop*, graph::VertexId> ptr_stop_id_edge_to_;
		std::unique_ptr<graph::Router<double>> router_u_ptr_;

		double CalculateWeight(const int distance);
		IdEgeInfoForPrint GetEdgeInfoForPrint(const graph::EdgeId id) const;

		void CreateEdgeFromAndToByStops(std::vector<Stop*> all_stops_ptr_);
		void MakeEdgeBus(const Bus* ptr_bus);

	};
}// router::
