// Copyright (c) 2018 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef MINIMUMSPANNINGTREE_H
#define MINIMUMSPANNINGTREE_H

#include "IntPoint.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cura
{

/*!
 * \brief Implements Prim's algorithm to compute Minimum Spanning Trees (MST).
 *
 * The minimum spanning tree is always computed from a clique of vertices.
 */
class MinimumSpanningTree
{
    /*!
     * \brief Represents an edge of the tree.
     *
     * While edges are meant to be undirected, these do have a start and end
     * point.
     */
    struct Edge
    {
        /**
         * The point at which this edge starts.
         */
        const Point start;

        /**
         * The point at which this edge ends.
         */
        const Point end;
    };

public:
    MinimumSpanningTree() = default;
    /*!
     * \brief Constructs a minimum spanning tree that spans all given vertices.
     */
    MinimumSpanningTree(std::vector<Point> vertices);

    /*!
     * \brief Gets the nodes that are adjacent to the specified node.
     * \return A list of nodes that are adjacent.
     */
    std::vector<Point> adjacentNodes(Point node) const;

    /*!
     * \brief Gets the leaves of the tree.
     * \return A list of nodes that are all leaves of the tree.
     */
    std::vector<Point> leaves() const;

    /*!
     * \brief Gets all vertices of the tree.
     * \return A list of vertices of the tree.
     */
    std::vector<Point> vertices() const;

private:
    using AdjacencyGraph_t = std::unordered_map<Point, std::vector<Edge>>;
    AdjacencyGraph_t adjacency_graph;

    /*!
     * \brief Computes the edges of a minimum spanning tree using Prim's
     * algorithm.
     *
     * \param vertices The vertices to span.
     * \return An adjacency graph with for each point one or more edges.
     */
    AdjacencyGraph_t prim(std::vector<Point> vertices) const;
};

} // namespace cura

#endif /* MINIMUMSPANNINGTREE_H */
