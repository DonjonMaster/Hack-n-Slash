#include <cassert>
#include <stdexcept>

#include "cpe/navigation.hpp"
namespace cpe {
    void AddUndirected(Graph& graph, int a, int b, double cost) 
    {
        // Précondition : a et b sont des indices valides.
        graph.at(a).push_back({ b, cost });
        graph.at(b).push_back({ a, cost });
    }
    Polygon MakePolygon(std::vector<Vec2> vertices)
    {
        if(vertices.size() < 3)
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

        result.center = {xsum / vertices.size(), zsum / vertices.size() };
        return result;
    }

    void NavMesh::CreatePortals()
    {
        for (int i = 0; i < polygons_.size(); i++)
        {
            for (int j = 0; j < polygons_.size(); j++)
            {
                if(j != i)
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
                                        for (auto &p : portals_)
                                        {
                                            if ((p.a == i || p.b == i) && (p.a == j || p.b == j))
                                                isPortalCreated = true;
                                        }
                                        if(!isPortalCreated)
                                        {
                                            Portal portal = { .a = i, .b = j,.p = polygons_[i].vertices[a],.q = polygons_[i].vertices[b] };
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

    bool NavMesh::Build() {
        graph_ = {};
        for (auto vertices : source_)
        {
            MakePolygon(vertices);
            graph_.push_back({});
        }
        CreatePortals();
        
        Vec2 middle;
        double cost;
        for (auto &portal : portals_) {
            //les portail.a et portail.b doivent correspondre à des régions différentes
            if(portal.a != portal.b)return false;
            //les portails doivent correspondre à des regions existante
            if(!(portal.a < 0 || portal.b < 0 || portal.a >= polygons_.size() || portal.b >= polygons_.size()))return false;
            //un portail ne peut pas etre 2 fois le meme point
            if(Length(portal.p - portal.q) != 0.0)return false;
            //les regions donner doivent correspondre au point donné
            if(
                (Contains(portal.a, portal.p) || Contains(portal.b, portal.p)) 
                &&
                (Contains(portal.a, portal.q) || Contains(portal.b, portal.q))
                )return false;
            
            middle = portal.Middle();
            cost = Cost(polygons_[portal.a], polygons_[portal.b], middle);
            AddUndirected(graph_, portal.a, portal.b, cost);
        }
        return true;
    }
int NavMesh::Locate(Vec2 point) const {
    for (int i = 0; i < polygons_.size(); i++)
    {
        if (Contains(i, point))
            return i;
    }
    return -1;
}
    bool NavMesh::Contains(int polygon, Vec2 point) const 
    {
        if(!Finite(point))
            throw std::invalid_argument("message");
        bool positive = false;
        bool negative = false;
        Polygon poly = polygons_[polygon];
        for (int i = 0; i < poly.vertices.size();i++) {
            //crossResult donne de quelle cote est notre point P
            double crossResult = Cross(poly.vertices[(i + 1) % poly.vertices.size()] - poly.vertices[i], point - poly.vertices[i]);
            if (crossResult > 1e-8)
                positive = true;
            else if (crossResult < -1e-8)
                negative = true;


            if (positive && negative)
                return false;
        }
        return true;
    }
bool NavMesh::SegmentInside(Vec2 from, Vec2 to) const {
    // TODO J3 : vérifier qu'un segment reste dans la surface navigable.
    (void)from;
    (void)to;
    return false;
}
Path NavMesh::FindPath(Vec2 from, Vec2 to, Algorithm algorithm) const {
    // TODO J3 : relier localisation, recherche sur graphe et trajet géométrique.
    (void)from;
    (void)to;
    (void)algorithm;
    return {};
}
}
