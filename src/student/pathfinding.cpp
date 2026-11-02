#include <iostream>
#include <ostream>
#include <queue>

#include "cpe/navigation.hpp"
namespace cpe {
    
    bool ValidNode(const Graph& graph, int id) {
        return id >= 0 && id < static_cast<int>(graph.size());
    }
    bool ValidGraph(const Graph& graph) {
        for (const auto& neighbors : graph) {
            for (const Edge& edge : neighbors) {
                if (!ValidNode(graph, edge.to) ||
                    !std::isfinite(edge.cost) || edge.cost < 0.0) {
                    return false;
                    }
            }
        }
        return true;
    }

    std::vector<double> DefineGraphHeuristic(const Graph& graph , Heuristic heuristic)
    {
        std::vector<double> h(graph.size());
        for (int i = 0; i < static_cast<int>(graph.size()); ++i) {
            h[i] = heuristic(i);
        }
        return h;
    }

    SearchResult Search(const Graph& graph, int start, int goal, const std::vector<double>& heuristic) {
        SearchResult result;
        if (!ValidGraph(graph) || !ValidNode(graph, start) || !ValidNode(graph, goal) || heuristic.size() != graph.size())
        {
            result.status = Status::InvalidInput;
            return result;
        }
            

        //on vérifie que l'heuritic n'a pas de valeur négative
        for (auto h : heuristic)
        {
            if (!std::isfinite(h) || h < 0.0) return result;
        }
        if (std::abs(heuristic[goal]) > 1e-9) return result;

        const double infinity = std::numeric_limits<double>::infinity();
        std::vector<double> g(graph.size(), infinity);
        std::vector<int> parent(graph.size(), -1);
        // Ordre de comparaison : f, puis g, puis identifiant.
        using Entry = std::tuple<double, double, int>;
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;
        g[start] = 0.0;
        open.push({ heuristic[start], 0.0, start });
        while (!open.empty()) {
            const auto [priority, savedG, u] = open.top();
            open.pop();
            (void)priority;
            if (savedG > g[u]) continue;
            if (u == goal) {
                result.status = Status::Found;
                result.cost = g[goal];
                for (int v = goal; v != -1; v = parent[v]) result.nodes.push_back(v);
                std::reverse(result.nodes.begin(), result.nodes.end());
                return result;
            }
            ++result.expanded;
            for (const Edge& edge : graph[u]) {
                const double candidate = g[u] + edge.cost;
                if (candidate < g[edge.to]) {
                    g[edge.to] = candidate;
                    parent[edge.to] = u;
                    open.push({ candidate + heuristic[edge.to], candidate, edge.to });
                }
            }
        }
        result.status = Status::NoPath;
        return result;
    }
    SearchResult Dijkstra(const Graph& graph, int start, int goal) {
        return Search(graph, start, goal, std::vector<double>(graph.size(), 0.0));
    }
    SearchResult AStar(const Graph& graph, int start, int goal,
                   const std::vector<double>& heuristic) {
        return Search(graph, start, goal, heuristic);
    }
}
