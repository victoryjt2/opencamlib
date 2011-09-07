/*  
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <sstream>
#include <string>

#include "weave.hpp"


namespace ocl
{

namespace weave
{

int VertexProps::count = 0;

Weave::~Weave() { // this special descrutctor required because hedi inherits from BGL::adjacency_list. FIXME.
    VertexItr vi, vi_end, next;
    boost::tie( vi, vi_end ) = boost::vertices( g );
    for ( next=vi ; vi != vi_end ; vi=next ) {
        ++next; // move next out of the way so it is not invalidated on the next line
        hedi::delete_vertex( *vi, g);
    }
}
        

void Weave::addFiber(Fiber& f) {
    if ( f.dir.xParallel() && !f.empty() ) {
        xfibers.push_back(f);
    } else if ( f.dir.yParallel() && !f.empty() ) {
        yfibers.push_back(f);
    } else if (!f.empty()) {
        assert(0); // fiber must be either x or y
    }
}

// traverse the graph putting loops of vertices into the loops variable
// this figure illustrates next-pointers: http://www.anderswallin.net/wp-content/uploads/2011/05/weave2_zoom.png
void Weave::face_traverse() { 
    std::cout << " traversing graph with " << clVertexSet.size() << " cl-points\n";
    while ( !clVertexSet.empty() ) { // while unprocessed cl-vertices remain
        std::vector<Vertex> loop; // start on a new loop
        Vertex current = *(clVertexSet.begin());
        Vertex first = current;
        
        do { // traverse around the loop
            assert( g[current].type == CL ); // we only want cl-points in the loop
            loop.push_back(current);
            clVertexSet.erase(current); // remove from set of unprocesser cl-verts
            std::vector<Edge> outEdges = hedi::out_edges(current, g); // find the edge to follow
            //if (outEdges.size() != 1 )
            //    std::cout << " outEdges.size() = " << outEdges.size() << "\n";
            assert( outEdges.size() == 1 ); // cl-points are allways at ends of intervals, so they have only one out-edge
            Edge currentEdge = outEdges[0]; 
            do { // following next, find a CL point 
                current = hedi::target( currentEdge, g);
                currentEdge = g[currentEdge].next;
            } while ( g[current].type != CL );
        } while (current!=first); // end the loop when we arrive at the start
        
        loops.push_back(loop); // add the processed loop to the master list of all loops
    }
}


std::vector< std::vector<Point> > Weave::getLoops() const {
    std::vector< std::vector<Point> > loop_list;
    BOOST_FOREACH( std::vector<Vertex> loop, loops ) {
        std::vector<Point> point_list;
        BOOST_FOREACH( Vertex v, loop ) {
            point_list.push_back( g[v].position );
        }
        loop_list.push_back(point_list);
    }
    return loop_list;
}


// this can cause a build error when both face and vertex descriptors have the same type
// i.e. unsigned int (?)
// operator[] below "g[*itr].type" then looks for FaceProps.type which does not exist...
void Weave::printGraph() const {
    std::cout << " number of vertices: " << boost::num_vertices( g ) << "\n";
    std::cout << " number of edges: " << boost::num_edges( g ) << "\n";
    VertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    int n=0, n_cl=0, n_internal=0;
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( g[*itr].type == CL )
            ++n_cl;
        else
            ++n_internal;
        ++n;
    }
    std::cout << " counted " << n << " vertices\n";
    std::cout << "          CL-nodes: " << n_cl << "\n";
    std::cout << "    internal-nodes: " << n_internal << "\n";
}
        
// string representation
std::string Weave::str() const {
    std::ostringstream o;
    o << "Weave2\n";
    o << "  " << xfibers.size() << " X-fibers\n";
    o << "  " << yfibers.size() << " Y-fibers\n";
    return o.str();
}
        

} // end weave namespace

} // end ocl namespace
// end file weave.cpp
