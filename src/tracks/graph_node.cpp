//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, B

#include "tracks/quad_graph.hpp"

#include "user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "tracks/quad_graph.hpp"
#include "tracks/quad_set.hpp"


// A static variable that gives a single graph node easy access to 
// all quads and avoids unnecessary parameters in many calls.
QuadSet *GraphNode::m_all_quads=NULL;

// This static variable gives a node access to the graph, and therefore
// to the quad to which a graph node index belongs.
QuadGraph *GraphNode::m_all_nodes=NULL;

// ----------------------------------------------------------------------------
/** Constructor. Saves the quad index which belongs to this graph node.
 *  \param index Index of the quad to use for this node (in m_all_quads).
 */
GraphNode::GraphNode(unsigned int index) 
{ 
    assert(index<m_all_quads->getNumberOfQuads());
    m_index               = index; 
    m_distance_from_start = 0;
    const Quad &quad      = m_all_quads->getQuad(m_index);
    // FIXME: those two values should probably depend on the actual 
    // orientation of the quad.
    // The width is the average width at the beginning and at the end.
    m_width = (  (quad[1]-quad[0]).length() 
               + (quad[3]-quad[2]).length() ) * 0.5f;
    Vec3 lower = (quad[0]+quad[1]) * 0.5f;
    Vec3 upper = (quad[2]+quad[3]) * 0.5f;
    m_line     = core::line2df(lower.getX(), lower.getY(),
                               upper.getX(), upper.getY() );
    // Only this 2d point is needed later
    m_lower_center = core::vector2df(lower.getX(), lower.getY());
}   // GraphNode

// ----------------------------------------------------------------------------
/** Adds a successor to a node. This function will also pre-compute certain
 *  values (like distance from this node to the successor, angle (in world)
 *  between this node and the successor.
 *  \param to The index of the successor.
 */
void GraphNode::addSuccessor(unsigned int to)
{
    m_vertices.push_back(to);
    // m_index is the quad index, so we use m_all_quads
    const Quad &this_quad = m_all_quads->getQuad(m_index);
    // to is the graph node, so we have to use m_all_nodes to get the right quad
    const Quad &next_quad = m_all_nodes->getQuad(to);

    Vec3 diff     = next_quad.getCenter() - this_quad.getCenter();
    m_distance_to_next.push_back(diff.length());
    
    float theta = -atan2(diff.getX(), diff.getY());
    m_angle_to_next.push_back(theta);

    // The length of this quad is the average of the left and right side
    float distance_to_next = (   this_quad[2].distance(this_quad[1])
                               + this_quad[3].distance(this_quad[0]) ) *0.5f;
    // The distance from start for the successor node 
    m_all_nodes->getNode(to).m_distance_from_start =
        std::max(m_all_nodes->getNode(to).m_distance_from_start, 
                 m_distance_from_start+distance_to_next);
}   // addSuccessor
// ----------------------------------------------------------------------------
/** Returns the distance a point has from this quad in forward and sidewards
 *  direction, i.e. how far forwards the point is from the beginning of the 
 *  quad, and how far to the side from the line connecting the center points
 *  is it. All these computations are done in 2D only.
 *  \param xyz The coordinates of the point.
 *  \param result The X coordinate contains the sidewards distance, the
 *                y coordinate the forward distance.
 */
void GraphNode::getDistances(const Vec3 &xyz, Vec3 *result)
{
    core::vector2df xyz2d(xyz.getX(), xyz.getY());
    core::vector2df closest = m_line.getClosestPoint(xyz2d);
    if(m_line.getPointOrientation(xyz2d)>0)
        result->setX( (closest-xyz2d).getLength());   // to the right
    else
        result->setX(-(closest-xyz2d).getLength());   // to the left
    result->setY( (closest-m_lower_center).getLength());
}   // getDistances
