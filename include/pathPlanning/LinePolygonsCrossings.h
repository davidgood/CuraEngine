// Copyright (c) 2018 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef PATH_PLANNING_LINE_POLYGONS_CROSSINGS_H
#define PATH_PLANNING_LINE_POLYGONS_CROSSINGS_H

#include "../utils/polygon.h"
#include "../utils/polygonUtils.h"
#include "CombPath.h"

namespace cura
{

/*!
 * Class for generating a combing move action from point a to point b and avoiding collision with other parts when moving through air.
 * See LinePolygonsCrossings::comb.
 *
 * The general implementation is by rotating everything such that the the line segment from a to b is aligned with the x-axis.
 * We call the line on which a and b lie the 'scanline'.
 *
 * The basic path is generated by following the scanline until it hits a polygon, then follow the polygon until the last point where it hits the scanline,
 * follow the scanline again, etc.
 * The path is offsetted from the polygons, so that it doesn't intersect with them.
 *
 * Next the basic path is optimized by taking shortcuts where possible. Only shortcuts which skip a single point are considered, in order to reduce computational complexity.
 */
class LinePolygonsCrossings
{
private:
    /*!
     * A Crossing holds data on a single point where a polygon crosses the scanline.
     */
    struct Crossing
    {
        size_t poly_idx; //!< The index of the polygon which crosses the scanline
        coord_t x; //!< x coordinate of crossings between the polygon and the scanline.
        size_t point_idx; //!< The index of the first point of the line segment which crosses the scanline

        /*!
         * Creates a Crossing with minimal initialization
         * \param poly_idx The index of the polygon in LinePolygonsCrossings::boundary
         * \param x The x-coordinate in transformed space
         * \param point_idx The index of the first point of the line segment which crosses the scanline
         */
        Crossing(const size_t poly_idx, const coord_t x, const size_t point_idx);
    };

    std::vector<Crossing> crossings; //!< All crossings of polygons in the LinePolygonsCrossings::boundary with the scanline.

    const Polygons& boundary; //!< The boundary not to cross during combing.
    LocToLineGrid& loc_to_line_grid; //!< Mapping from locations to line segments of \ref LinePolygonsCrossings::boundary
    Point startPoint; //!< The start point of the scanline.
    Point endPoint; //!< The end point of the scanline.

    int64_t dist_to_move_boundary_point_outside; //!< The distance used to move outside or inside so that a boundary point doesn't intersect with the boundary anymore. Neccesary
                                                 //!< due to computational rounding problems. Use negative value for insicde combing.

    PointMatrix transformation_matrix; //!< The transformation which rotates everything such that the scanline is aligned with the x-axis.
    Point transformed_startPoint; //!< The LinePolygonsCrossings::startPoint as transformed by Comb::transformation_matrix such that it has (roughly) the same Y as
                                  //!< transformed_endPoint
    Point
        transformed_endPoint; //!< The LinePolygonsCrossings::endPoint as transformed by Comb::transformation_matrix such that it has (roughly) the same Y as transformed_startPoint


    /*!
     * Check if we are crossing the boundaries, and pre-calculate some values.
     *
     * Sets Comb::transformation_matrix, Comb::transformed_startPoint and Comb::transformed_endPoint
     * \return Whether the line segment from LinePolygonsCrossings::startPoint to LinePolygonsCrossings::endPoint collides with the boundary
     */
    bool lineSegmentCollidesWithBoundary();

    /*!
     * Calculate Comb::crossings.
     * \param fail_on_unavoidable_obstacles When moving over other parts is inavoidable, stop calculation early and return false.
     * \return Whether combing succeeded, i.e. when fail_on_unavoidable_obstacles: we didn't cross any gaps/other parts
     */
    bool calcScanlineCrossings(bool fail_on_unavoidable_obstacles);

    /*!
     * Generate the basic combing path and optimize it.
     *
     * \param combPath Output parameter: the points along the combing path.
     * \param fail_on_unavoidable_obstacles When moving over other parts is inavoidable, stop calculation early and return false.
     * \return Whether combing succeeded, i.e. we didn't cross any gaps/other parts
     */
    bool generateCombingPath(CombPath& combPath, int64_t max_comb_distance_ignored, bool fail_on_unavoidable_obstacles);

    /*!
     * Generate the basic combing path, without shortcuts. The path goes straight toward the endPoint and follows the boundary when it hits it, until it passes the scanline again.
     *
     * Walk trough the crossings, for every boundary we cross, find the initial cross point and the exit point. Then add all the points in between
     * to the \p combPath and continue with the next boundary we will cross, until there are no more boundaries to cross.
     * This gives a path from the start to finish curved around the holes that it encounters.
     *
     * \param combPath Output parameter: the points along the combing path.
     */
    void generateBasicCombingPath(CombPath& combPath);

    /*!
     * Generate the basic combing path, following a single boundary polygon when it hits it, until it passes the scanline again.
     *
     * Find the initial cross point and the exit point. Then add all the points in between
     * to the \p combPath and continue with the next boundary we will cross, until there are no more boundaries to cross.
     * This gives a path from the start to finish curved around the polygon that it encounters.
     *
     * \param combPath Output parameter: where to add the points along the combing path.
     */
    void generateBasicCombingPath(const Crossing& min, const Crossing& max, CombPath& combPath);

    /*!
     * Optimize the \p comb_path: skip each point we could already reach by not crossing a boundary. This smooths out the path and makes it skip some unneeded corners.
     *
     * \param comb_path The unoptimized combing path.
     * \param optimized_comb_path Output parameter: The points of optimized combing path
     * \return Whether it turns out that the basic comb path already crossed a boundary
     */
    bool optimizePath(CombPath& comb_path, CombPath& optimized_comb_path);

    /*!
     * Create a LinePolygonsCrossings with minimal initialization.
     * \param boundary The boundary which not to cross during combing
     * \param start the starting point
     * \param end the end point
     * \param dist_to_move_boundary_point_outside Distance used to move a point from a boundary so that it doesn't intersect with it anymore. (Precision issue)
     */
    LinePolygonsCrossings(const Polygons& boundary, LocToLineGrid& loc_to_line_grid, Point& start, Point& end, int64_t dist_to_move_boundary_point_outside)
        : boundary(boundary)
        , loc_to_line_grid(loc_to_line_grid)
        , startPoint(start)
        , endPoint(end)
        , dist_to_move_boundary_point_outside(dist_to_move_boundary_point_outside)
    {
    }

public:
    /*!
     * The main function of this class: calculate one combing path within the boundary.
     * \param boundary The polygons to follow when calculating the basic combing path
     * \param loc_to_line_grid A sparse grid mapping cells to all line segments of (at least) \p boundary in those cells
     * \param startPoint From where to start the combing move.
     * \param endPoint Where to end the combing move.
     * \param combPath Output parameter: the combing path generated.
     * \param fail_on_unavoidable_obstacles When moving over other parts is inavoidable, stop calculation early and return false.
     * \return Whether combing succeeded, i.e. we didn't cross any gaps/other parts
     */
    static bool comb(
        const Polygons& boundary,
        LocToLineGrid& loc_to_line_grid,
        Point startPoint,
        Point endPoint,
        CombPath& combPath,
        int64_t dist_to_move_boundary_point_outside,
        int64_t max_comb_distance_ignored,
        bool fail_on_unavoidable_obstacles)
    {
        LinePolygonsCrossings linePolygonsCrossings(boundary, loc_to_line_grid, startPoint, endPoint, dist_to_move_boundary_point_outside);
        return linePolygonsCrossings.generateCombingPath(combPath, max_comb_distance_ignored, fail_on_unavoidable_obstacles);
    };
};

} // namespace cura

#endif // PATH_PLANNING_LINE_POLYGONS_CROSSINGS_H
