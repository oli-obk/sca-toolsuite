/*************************************************************************/
/* sca toolsuite - a toolsuite to simulate cellular automata.            */
/* Copyright (C) 2011-2014                                               */
/* Johannes Lorenz                                                       */
/* https://github.com/JohannesLorenz/sca-toolsuite                       */
/*                                                                       */
/* This program is free software; you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation; either version 3 of the License, or (at */
/* your option) any later version.                                       */
/* This program is distributed in the hope that it will be useful, but   */
/* WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      */
/* General Public License for more details.                              */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program; if not, write to the Free Software           */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA  */
/*************************************************************************/

#ifndef RESULTS_H
#define RESULTS_H

#include <set>
#include "types.h"
#include "io/serial.h"

enum class exactness_t // TODO: use mapper class
{
	equal,
	greater_equal
}; // TODO: enum class?

//! one result configuration
class rec_pair : types
{
public:

	static const char* get_exactness_string(exactness_t ec)
	{
		switch(ec)
		{
			case exactness_t::equal: return "=";
			case exactness_t::greater_equal: return ">=";
			default: return nullptr;
		}
	}

	std::set<point> points; //!< probably: points ever changed since start, >= patch.area()
	patch_t result; //!< result patch
	std::size_t scc_size = 1; //!< minimum scc size
	exactness_t exactness; //!< how exact scc_size is
//	std::set<point> custom_points;

	friend std::ostream& operator<< (std::ostream& stream,
		const rec_pair& r);

	void print_pretty(grid_t& tmp_grid, std::ostream& stream = std::cerr) const;

	rec_pair(std::set<point>&& points,
		patch_t&& result,
		std::size_t scc_size = 1,
		exactness_t exactness = exactness_t::equal) :
		points(points),
		result(result),
		scc_size(scc_size),
		exactness(exactness)
	{}

	rec_pair()
	{}

	//! the typical comparison
	bool operator==(const rec_pair& rhs) const
	{
		return (rhs.points == points)
			&& (rhs.result == result)
			&& (rhs.scc_size == scc_size); // TODO?
	}

	bool operator<(const rec_pair& rhs) const
	{
		// TODO: is comparing results first faster?
		return points < rhs.points
			|| (!(rhs.points < points) && result < rhs.result);
	}

	friend io::serializer& operator<<(io::serializer& s, const rec_pair& r) {
		return s << r.points << r.result << r.scc_size << r.exactness; }
	friend io::deserializer& operator>>(io::deserializer& s, rec_pair& r) {
		return s >> r.points >> r.result >> r.scc_size >> r.exactness; }


};

//! structure that keeps track of all results
struct rec_rval_base_t : types
{
	std::set<point> total_area; //!< area of all cells below us in the tree
	std::set<rec_pair> final_candidates; //!< end configurations

	friend std::ostream& operator<< (std::ostream& stream,
		const rec_rval_base_t& rval);

	friend io::serializer& operator<<(io::serializer& s, const rec_rval_base_t& r) {
		return s << r.total_area << r.final_candidates; }
	friend io::deserializer& operator>>(io::deserializer& s, rec_rval_base_t& r) {
		return s >> r.total_area >> r.final_candidates; }

	void print_results_pretty(const grid_t &orig_grid) const;
};

class result_t : types
{
public:
	std::size_t id;
	std::size_t vid;
	std::size_t scc_size;
	exactness_t exactness;
	patch_t local_patch;
	std::vector<result_t*> children;

	void dump(std::ostream& os, std::size_t depth = 0) const
	{
		//if(used())
		for(std::size_t d = 0; d < depth; ++d)
		 os << ' ';
		os << local_patch;
		for(const result_t* r : children)
		 r->dump(os, depth + 1);
		os << std::endl;
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const result_t& r)
	{
		return r.dump(stream), stream;
	}

	friend io::serializer& operator<<(io::serializer& s, const result_t& r)
	{
		s << r.id << r.vid << r.scc_size << r.exactness << r.local_patch;
		s << (uint64_t)r.children.size();
		//std::cerr << "r.id: " << r.id << ", ch: " << r.children.size() << std::endl;
		for(const result_t* res : r.children)
		{
			s << res->id;
		}
		return s;
	}

	friend io::deserializer& operator>>(io::deserializer& s, result_t& r)
	{
		return s >> r.id >> r.vid >> r.scc_size >> r.exactness >> r.local_patch;
	}
};

//#define SERIAL_DEBUG // <- sca toolsuite must do this
//! a very simple tree-like graph, avoiding boost::graph
template<class vertex_t>
struct m_graph_t
{
	std::vector<vertex_t*> vertices;
	std::vector<vertex_t*> roots;

	template<class ...Args>
	vertex_t*& add_vertex(Args... args) {
		//vertices.emplace_back(args...);
		vertex_t* ptr = new vertex_t(args...);
		ptr->id = vertices.size();
		vertices.push_back(ptr);
		return vertices.back();
	}

	~m_graph_t() { /*for(const vertex_t* v : vertices)
		delete v;*/ } // TODO!

	friend io::serializer& operator<<(io::serializer& s, const m_graph_t& g)
	{
		uint64_t sz = g.vertices.size();
		s << sz;
#ifdef SERIAL_DEBUG
		std::cerr << "Writing res_graph of size " << sz << std::endl;
#endif
		for(const vertex_t* v : g.vertices)
		 s << *v;
		s << (uint64_t)g.roots.size();
#ifdef SERIAL_DEBUG
		std::cerr << "Writing " << g.roots.size() << " roots..." <<  std::endl;
#endif
		for(const vertex_t* v : g.roots)
		 s << v->id;
		return s;
	}

	friend io::deserializer& operator>>(io::deserializer& s, m_graph_t& g)
	{
		uint64_t sz;
		s >> sz;
#ifdef SERIAL_DEBUG
		std::cerr << "Reading res_graph of size " << sz << std::endl;
#endif
		g.vertices.resize(sz);
		std::size_t id = 0;
		for(vertex_t*& v : g.vertices)
		{
			v = new vertex_t();
		}

		for(vertex_t*& v : g.vertices)
		{
			s >> v;
			v->id = id++;
			s >> sz;
			v->children.resize(sz);
			for(std::size_t i = 0; i < sz; ++i)
			{
				std::size_t cid;
				s >> cid;
				v->children[i] = g.vertices[cid];
			}
		}

		s >> sz;
		g.roots.resize(sz);
#ifdef SERIAL_DEBUG
		std::cerr << "Reading " << sz << " roots..." <<  std::endl;
#endif
		for(vertex_t*& v : g.roots)
		{
			s >> id;
			v = g.vertices[id];
		}
		return s;
	}

};

#endif // RESULTS_H
