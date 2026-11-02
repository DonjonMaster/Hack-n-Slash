#include <cassert>
#include <stdexcept>

#include "cpe/navigation.hpp"

namespace cpe
{
    void AddUndirected(Graph& graph, int a, int b, double cost)
    {
        // Précondition : a et b sont des indices valides.
        graph.at(a).push_back({b, cost});
        graph.at(b).push_back({a, cost});
    }

    Polygon MakePolygon(std::vector<Vec2> vertices)
    {
        if (vertices.size() < 3)
            throw std::invalid_argument("message");

        Polygon result;
        double xsum = 0;
        double zsum = 0;
        for (int i = 0; i < vertices.size(); i++)
        {
            for (int j = 0; j < vertices.size(); j++)
            {
                if (i != j && Near(vertices[i], vertices[j]))
                    throw std::invalid_argument("same point");
            }
            if (Finite(vertices[i]))
            {
                result.vertices.push_back(vertices[i]);
                xsum += vertices[i].x;
                zsum += vertices[i].z;
            }
            else
                throw std::invalid_argument("inFinite Point");
        }

        result.center = {xsum / vertices.size(), zsum / vertices.size()};
        return result;
    }


    const Portal* NavMesh::Between(int a, int b)const
    {
        for (auto &portal : portals_)
        {
            if (portal.a == a && portal.b == b)
                return &portal;
        }
        return nullptr;
    }

    void NavMesh::CreatePortals()
    {
        for (int i = 0; i < polygons_.size(); i++)
        {
            for (int j = 0; j < polygons_.size(); j++)
            {
                if (j != i)
                {
                    for (int a = 0; a < polygons_[i].vertices.size(); a++)
                    {
                        if (Contains(j, polygons_[i].vertices[a]))
                        {
                            for (int b = 0; b < polygons_[i].vertices.size(); b++)
                            {
                                if (a != b)
                                {
                                    if (Contains(j, polygons_[i].vertices[b]))
                                    {
                                        bool isPortalCreated = false;
                                        for (auto& p : portals_)
                                        {
                                            if ((p.a == i || p.b == i) && (p.a == j || p.b == j))
                                                isPortalCreated = true;
                                        }
                                        if (!isPortalCreated)
                                        {
                                            Portal portal = {
                                                .a = i, .b = j, .p = polygons_[i].vertices[a],
                                                .q = polygons_[i].vertices[b]
                                            };
                                            portals_.push_back(portal);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    bool NavMesh::Build()
    {
        graph_ = {};
        for (auto vertices : source_)
        {
            polygons_.push_back(MakePolygon(vertices));
            graph_.push_back({});
        }
        CreatePortals();

        Vec2 middle;
        double cost;
        for (auto& portal : portals_)
        {
            //les portail.a et portail.b doivent correspondre à des régions différentes
            if (portal.a == portal.b)return false;
            //les portails doivent correspondre à des regions existante
            if (portal.a < 0 || portal.b < 0 || portal.a >= polygons_.size() || portal.b >= polygons_.size())return
                false;
            //un portail ne peut pas etre 2 fois le meme point
            if (Length(portal.p - portal.q) == 0.0)return false;
            //les regions donner doivent correspondre au point donné
            if (
                !(Contains(portal.a, portal.p) || Contains(portal.b, portal.p))
                &&
                !(Contains(portal.a, portal.q) || Contains(portal.b, portal.q))
            )
                return false;

            middle = portal.Middle();
            cost = Cost(polygons_[portal.a], polygons_[portal.b], middle);
            AddUndirected(graph_, portal.a, portal.b, cost);
        }
        built_ = true;
        return true;
    }

    int NavMesh::Locate(Vec2 point) const
    {
        for (int i = 0; i < polygons_.size(); i++)
        {
            if (Contains(i, point))
                return i;
        }
        return -1;
    }

    bool NavMesh::Contains(int polygon, Vec2 point) const
    {
        if (!Finite(point))
            throw std::invalid_argument("message");
        bool positive = false;
        bool negative = false;
        Polygon poly = polygons_[polygon];
        for (int i = 0; i < poly.vertices.size(); i++)
        {
            //crossResult donne de quelle cote est notre point P
            double crossResult = Cross(poly.vertices[(i + 1) % poly.vertices.size()] - poly.vertices[i],
                                       point - poly.vertices[i]);
            if (crossResult > 1e-8)
                positive = true;
            else if (crossResult < -1e-8)
                negative = true;


            if (positive && negative)
                return false;
        }
        return true;
    }

    bool NavMesh::SegmentInside(Vec2 from, Vec2 to) const
    {
        if (!Finite(from) || !Finite(to)) return false;
        std::vector<std::pair<double, double>> intervals;
        const Vec2 direction = to - from;
        for (const Polygon& polygon : polygons_)
        {
            const auto& v = polygon.vertices;
            double orientation = Cross(v[1] - v[0], v[2] - v[1]) > 0 ? 1.0 : -1.0;
            double low = 0.0, high = 1.0;
            bool valid = true;
            for (std::size_t i = 0; i < v.size(); ++i)
            {
                Vec2 edge = v[(i + 1) % v.size()] - v[i];
                double base = orientation * Cross(edge, from - v[i]);
                double slope = orientation * Cross(edge, direction);
                // base + slope*t >= -epsilon
                if (std::abs(slope) < 1e-12)
                {
                    if (base < -1e-9)
                    {
                        valid = false;
                        break;
                    }
                }
                else
                {
                    double bound = (-1e-9 - base) / slope;
                    if (slope > 0) low = std::max(low, bound);
                    else high = std::min(high, bound);
                }
            }
            if (valid && low <= high) intervals.push_back({low, high});
        }
        std::sort(intervals.begin(), intervals.end());
        double covered = 0.0;
        for (auto [low,high] : intervals)
        {
            if (low > covered + 1e-8) return false;
            covered = std::max(covered, high);
            if (covered >= 1.0 - 1e-8) return true;
        }
        return false;
    }

    Path NavMesh::FindPath(Vec2 from, Vec2 to, Algorithm algorithm) const
    {
    if (!Finite(from) || !Finite(to))
        throw std::invalid_argument("invalid Input");
    if (Near(from, to))
    {
        SearchResult searchResult = { .status = Status::Found, .cost = 0 };
        Path result = { .search = searchResult,.points = {from}, .length = 0 };
        return result;
    }
    int startRegion = Locate(from);
    int goalRegion = Locate(to);
    if (startRegion == -1 || goalRegion == -1)
    {
        SearchResult searchResult = { .status = Status::OutsideNavMesh};
        Path result = { .search = searchResult };
        return result;
    }
    else if (startRegion == goalRegion)
    {
        SearchResult searchResult = { .status = Status::Found, .cost = 0 };
        auto delta = to - from;
        Path result = {.search = searchResult,.points = {from, to}, .length = sqrt(delta.x * delta.x + delta.z * delta.z) };
        return result;
    }


    SearchResult searchResult;
    if (algorithm == Algorithm::AStar)
    {
        std::vector<double> heuristic = DefineGraphHeuristic(graph_, [&](int n)->double {
        double dx = n % graph_.size() - goalRegion % graph_.size();
        double dy = n / graph_.size() - goalRegion / graph_.size();
        return sqrt(dx * dx + dy * dy);
        });
        searchResult = AStar(graph_, startRegion, goalRegion, heuristic);
    }
    else
        searchResult = Dijkstra(graph_, startRegion, goalRegion);


    Path pathResult = { .search = searchResult };
    if (!(searchResult.status == Status::Found))
    {
        return pathResult;
    }



    //if(!SamePoint(start, mesh.polygons[searchResult.nodes[0]].center))
    pathResult.points.push_back(from);
    for (int i = 0; i < searchResult.nodes.size()-1; i++)
    {
        //pathResult.points.push_back(mesh.polygons[searchResult.nodes[i]].center);
        auto portal = Between(searchResult.nodes[i], searchResult.nodes[i + 1]);
        if (portal)
            pathResult.points.push_back(portal->Middle());
    }
    //if(!SamePoint(start, mesh.polygons[searchResult.nodes[searchResult.nodes.size() - 1]].center))
    //    pathResult.points.push_back(mesh.polygons[searchResult.nodes[searchResult.nodes.size() - 1]].center);
    pathResult.points.push_back(to);


    pathResult.length = 0;
    for (int i = 0; i < pathResult.points.size() - 1; i++)
    {
        auto delta = pathResult.points[i + 1] - pathResult.points[i];
        pathResult.length += sqrt(delta.x * delta.x + delta.z * delta.z);
    }

    return pathResult;
}
}
