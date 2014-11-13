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

#ifndef BASE_GRAPH_H
#define BASE_GRAPH_H

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include "common_macros.h"
#include "detail.h"
#ifdef EXTENDED_GRAPH
	#include "utils/contract.h"
#endif
#include "utils/exceptions.h"
#include "io/serial.h"

template<class Vertex, class Edge, class OutEdgeList = boost::vecS, class VertexList = boost::vecS>
class graph_base_t
{
public:
	using self = graph_base_t<Vertex, Edge, OutEdgeList, VertexList>;
	using vertex = Vertex;
	using edge = Edge;
	typedef boost::adjacency_list<OutEdgeList, VertexList, boost::directedS, vertex, edge> graph_t;
protected:
	graph_t graph;
	std::string name;
	bool dump_on_exit;
public:
	typedef typename boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
	typedef typename boost::graph_traits<graph_t>::edge_descriptor edge_t;

	template<class ...Args>
	vertex_t add_vertex(Args... args)
	{
		vertex_t v_n = boost::add_vertex(graph);
		graph[v_n] = vertex(args...);
		return v_n;
	}

	template<class ...Args>
	edge_t add_edge(vertex_t v_1, vertex_t v_2, Args... args)
	{
		auto res = boost::add_edge(v_1, v_2, graph);
		if(!res.second)
		 throw "Could not add edge.";
		else
		{
			graph[res.first] = edge(args...);
			return res.first;
		}
	}


	static auto null_vertex()
		-> decltype(boost::graph_traits<graph_t>::null_vertex()) {
		return boost::graph_traits<graph_t>::null_vertex();
	}

private:
	// TODO: from stackoverflow
	static void my_replace(std::string& str, const std::string& oldStr, const std::string& newStr)
	{
		size_t pos = 0;
		while((pos = str.find(oldStr, pos)) != std::string::npos)
		{
			str.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
	}

	template<class VertexOrEdge>
	class newline_writer
	{
		const graph_t& g;
	public:
		newline_writer(const graph_t& g) : g(g) {}
		void operator()(std::ostream &out, const VertexOrEdge& ve) const
		{
			std::string res = g[ve].label;
			my_replace(res, "\n", "\\n");
			out << "[label=\"" << res << "\"]";
		}
	};
public:
	static std::ostream& _dump_as_dot(std::ostream& stream, graph_t& g)
	{
#ifdef USE_LABELS

		using index_map_t = std::map<vertex_t, size_t>;
		index_map_t index_map;
		boost::associative_property_map<index_map_t> idx(index_map);

		int i=0;
		for(const vertex_t& v : vertex_container_t(g))
		 put(idx, v, i++);

		write_graphviz(stream, g,
			newline_writer<vertex_t>(g),
			newline_writer<edge_t>(g),
			boost::default_writer(), idx
			);
#else
		(void)g;
#endif
		return stream;
	}
public:
	friend std::ostream& operator<<(std::ostream &stream, const graph_base_t<vertex, edge, OutEdgeList, VertexList> &d)
	{
		return _dump_as_dot(stream, const_cast<graph_t&>(d.graph));
	}

protected:
	struct vertex_container_t // TODO: common class for those three?
	{
		const graph_t& g;
		typedef typename boost::graph_traits<graph_t>::vertex_iterator iterator;
		using const_iterator = iterator;
		const_iterator cbegin() const { return boost::vertices(g).first; } // TODO: does cons make sense?
		const_iterator cend() const { return boost::vertices(g).second; }
		iterator begin() const { return boost::vertices(g).first; } // TODO: does cons make sense?
		iterator end() const { return boost::vertices(g).second; }
		vertex_container_t(const graph_t& g) : g(g) {}
	};

	struct edge_container_t
	{
		const graph_t& g;
		typedef typename boost::graph_traits<graph_t>::edge_iterator iterator;
		using const_iterator = iterator;
		iterator begin() const { return boost::edges(g).first; } // TODO: does cons make sense?
		iterator end() const { return boost::edges(g).second; }
		edge_container_t(const graph_t& g) : g(g) {}
	};

	struct out_edge_container_t
	{
		typedef typename boost::graph_traits<graph_t>::out_edge_iterator iterator;
		using const_iterator = iterator;
		std::pair<iterator, iterator> range;
		iterator begin() const { return range.first; } // TODO: does cons make sense?
		iterator end() const { return range.second; }
		bool empty() const { return begin() == end(); }
		out_edge_container_t(const vertex_t& v, const graph_t& g) : range(boost::out_edges(v, g)) {}
	};
public:
	class safe_out_edge_iterator_t
	{
		vertex_t src_vertex;
		const graph_t& graph;
		std::size_t max, cur_edge;
		using me = safe_out_edge_iterator_t;

		std::size_t get_max() const
		{
			return boost::out_degree(src_vertex, graph); // TODO?
		}
	public:
		const me& operator++()
		{
			cur_edge = std::min(cur_edge+1, max);
			return *this;
		}
		edge_t operator*() const
		{
			out_edge_container_t oec(src_vertex, graph);
			auto itr = oec.begin();
			for(std::size_t i = 0; i < cur_edge; ++i, ++itr) ;
			return *itr;
		}
		safe_out_edge_iterator_t(const vertex_t& v, const graph_t& g, bool begin) :
			src_vertex(v),
			graph(g),
			max(get_max()),
			cur_edge(begin ? 0 : max)
		{
		}
		//! note that this will not help against edges from different graphs
		bool operator==(const me& other) const
		{
			return cur_edge == other.cur_edge;
		}
		bool operator!=(const me& other) const
		{
			return ! operator==(other);
		}
		using value_type = edge_t;
	};

	struct safe_out_edge_container_t
	{
		using me = safe_out_edge_container_t;
		using iterator = safe_out_edge_iterator_t;
		using const_iterator = iterator;

		const vertex_t& v;
		const graph_t& g;

		safe_out_edge_container_t(const vertex_t& v, const graph_t& g) :
			v(v), g(g)
		{
		}

		iterator begin() const { return safe_out_edge_iterator_t(v,g,true); } // TODO: does cons make sense?
		iterator end() const { return safe_out_edge_iterator_t(v,g,false); }
		bool empty() { auto oe = boost::out_edges(v, g); return oe.first == oe.second; }
		std::size_t size() const { return boost::out_degree(v, g); }
	};

	//! returns second == true iff the edge could be added (i.e. was not existing)
	//! useful to avoid double edges
	std::pair<edge_t, bool> try_add_edge(const vertex_t& v1, const vertex_t& v2)
	{
#ifdef USE_VEC
		if(boost::num_vertices(g) <= std::max(v1, v2))
		{
			std::cout << boost::num_vertices(g) << ", " << v1 << ", " << v2 << std::endl;
			throw "add_edge: vertex does not exist.";
		}
#endif

		auto range = boost::out_edges(v1, graph);
		auto itr = range.first;
		for( ; itr != range.second; ++itr)
		if( boost::target(*itr, graph) == v2 )
		{
			break;
		}

		std::pair<edge_t, bool> ret_val;
		ret_val.second = (itr == range.second);
		ret_val.first = (ret_val.second)
			? boost::add_edge(v1, v2, graph).first
			: *itr;
		return ret_val;
	}

	std::size_t out_degree(const vertex_t& v) { return boost::out_degree(v, graph); }

	vertex_container_t vertices() const { return vertex_container_t(graph); }
	edge_container_t edges() const { return edge_container_t(graph); }
	safe_out_edge_container_t out_edges(const vertex_t& v) const { return safe_out_edge_container_t(v, graph); }

	std::size_t num_vertices() const { return boost::num_vertices(graph); }
	std::size_t num_edges() const { return boost::num_edges(graph); }
	vertex_t add_vertex() { return boost::add_vertex(graph); }
	std::pair<edge_t, bool> add_edge(const vertex_t& v1, const vertex_t& v2) { return boost::add_edge(v1, v2, graph); }
	vertex_t target(const edge_t& e) const { return boost::target(e, graph); }
	vertex_t source(const edge_t& e) const { return boost::source(e, graph); }

//	void remove_unused_out_edges(const vertex_t& v_cur, const std::vector<bool>& edge_used);

	void remove_unused_out_edges(
		const vertex_t& v_cur,
		const std::vector<bool>& edge_used)
	{
#ifdef REMOVE_ORPHANS
		std::size_t j = 0; // counts like i, but only if the edge will note be removed
		for(std::size_t i = 0; i < out_edges(v_cur).size(); ++i)
		{
			if(!edge_used[j])
			{
				auto itr = boost::out_edges(v_cur, graph).first;
				for(std::size_t k = 0; k < j; ++k) { ++itr; }
				 boost::remove_edge(itr, graph);
			}
			else
			 ++j;
		}
#else
		(void)v_cur;
		(void)edge_used;
#endif
	}

	void unconnected_removed(graph_t& res) const
	{
		std::set<vertex_t> keep;
		for(const edge_t& e : edges())
		{
			keep.insert(source(e));
			keep.insert(target(e));
		}

		::copy_subgraph(res, graph, keep);
	}


	//void notify_vertex_unused(const vertex_t& , const std::set<point>& ) {}
	template<class T> // TODO: ...
	void notify_vertex_unused(const vertex_t& , const T& ) {}

	void label_edge(const edge_t& e_n) {
		std::ostringstream ss;
		ss << graph[e_n];
		graph[e_n].label = ss.str();
	}
	void label_vertex(const vertex_t& v_n) {
		std::ostringstream ss;
		ss << graph[v_n];
		graph[v_n].label = ss.str();
	}

	std::set<vertex_t> remove_orphans(const vertex_t& v_root) {
#ifdef EXTENDED_GRAPH
		return ::remove_orphans(graph, v_root);
#else
		(void)v_root;
#endif
	}

	std::set<vertex_t> orphans_removed(graph_t& res, const vertex_t& v_root) const {
#ifdef EXTENDED_GRAPH
		return ::orphans_removed(res, graph, v_root);
#else
		(void)res; (void)v_root;
		throw up;
#endif
	}

	vertex& get(const vertex_t& v) { return graph[v]; }
	edge& get(const edge_t& e) { return graph[e]; }
	const vertex& get(const vertex_t& v) const { return graph[v]; }
	const edge& get(const edge_t& e) const { return graph[e]; }

	template<class Pred>
	void contract_vertices_inplace(const Pred& p) {
#ifdef EXTENDED_GRAPH
		::contract_vertices_inplace(graph, p);
#else
		(void)p;
		throw up;
#endif
	}

	template<class Pred>
	void contract_vertices(self& tar, const Pred& p, std::map<vertex_t, vertex_t> mapping) const {
#ifdef EXTENDED_GRAPH
		::contract_vertices(graph, tar.graph, p, mapping);
#else
		(void)tar; (void)p; (void)mapping;
		throw up;
#endif
	}

	template<class Container>
	void copy_vertex_list(graph_t& res, Container verts) const
	{
		const graph_t& gp = graph; // TODO

		std::map<vertex_t, vertex_t> new_id_of;
		for(const vertex_t& u : verts)
		{// TODO: store new id
			const vertex_t n = new_id_of[u] = boost::add_vertex(res);
			res[n] = gp[u];
		}

		auto es = boost::edges(gp);
		for(auto itr = es.first; itr != es.second; ++itr)
		{
			const vertex_t src = boost::source(*itr, gp), tar = boost::target(*itr, gp);
			const auto new_src = new_id_of.find(src), new_tar = new_id_of.find(tar);
			if(new_src != new_id_of.end() && new_tar != new_id_of.end())
			{
				const auto tmp = boost::add_edge(new_id_of[src], new_id_of[tar], res);
				res[tmp.first] = gp[*itr];
			}
		}
	}

	void copy_out(graph_t& res, const vertex_t& from, int depth) const
	{
#ifdef EXTENDED_GRAPH
		using color_map_t = std::map<vertex_t, boost::default_color_type>;
		color_map_t color_map;

		for(auto& pr : color_map)
		 pr.second = boost::white_color;

		boost::associative_property_map<color_map_t> pm_color(color_map);
		// TODO?

		int cur_depth = 0;

		// returns true if shall terminate
		auto term_func = [&](const vertex_t&, const graph_t&) { return cur_depth > depth; };

		std::vector<vertex_t> to_copy;

		auto cb_disc = [&](const vertex_t& v, const graph_t& )
		{
			to_copy.push_back(v);
			++cur_depth;
		};
		auto cb_fin = [&](const vertex_t& , const graph_t& )
		{
			--cur_depth;
		};

		dfs_struct_simplify<decltype(cb_disc), decltype(cb_fin)> dfs_vis(cb_disc, cb_fin);

		boost::depth_first_visit(graph, from, dfs_vis, pm_color, term_func);

		copy_vertex_list(res, to_copy);
#else
		(void)res; (void)from; (void)depth;
		throw up;
#endif
	}

	void remove_unconnected()
	{
		graph_t no_unconnected;
		unconnected_removed(no_unconnected);
		graph = no_unconnected;
	}

	graph_base_t(const char* name, bool dump_on_exit = true) : name(name),
		dump_on_exit(dump_on_exit) {}

	~graph_base_t()
	{
		if(dump_on_exit)
		{
			std::cerr << "dumping " << name << ".dot (" << num_vertices() << " vertices) ..." << std::endl;
#ifndef USE_LABELS
			std::cerr << "WARNING: debug graph unlabeled" << std::endl;
#endif
			//remove_orphans();
			//std::ofstream("debug_graph.dot") << (*this);
#ifdef REMOVE_ORPHANS
			remove_unconnected();
#endif
			std::ofstream ofs(name + ".dot");
			ofs << *this;
		}
	}
};

template<class Vertex, class Edge>
class debug_graph_t : public graph_base_t<Vertex, Edge, boost::vecS, boost::vecS>
{
/*	std::vector<vertex_t> useless;
public:
	void mark_unused(const vertex_t& v)
	{
		useless.push_back(v);
	}

	void cleanup_useless()
	{

	}*/
	//using self = debug_graph_t<Vertex, Edge>;
public:
	using base = graph_base_t<Vertex, Edge, boost::vecS, boost::vecS>;

#ifdef DEBUG_GRAPH
	using vertex_t = typename base::vertex_t;
#else
	struct vertex_t {};
#endif
private:
	bool dump_on_exit; //!< @todo unused
	vertex_t root; // TODO: extra class: rooted_debug_graph_t?
public:
	void cleanup_useless()
	{
		std::map<vertex_t, vertex_t> m;
		debug_graph_t g2;
		base::contract_vertices(
			g2, [&](const vertex_t& v){ return base::get(v).keep; }, m
		);
		*this = g2;
	}

//	std::set<vertex_t> remove_orphans() { return base::remove_orphans(root); }
	std::set<vertex_t> orphans_removed(typename base::graph_t& res) const {
		return base::orphans_removed(res, root); }
	std::pair<typename base::edge_t, bool>
	add_edge_from_root(const vertex_t& v2) { return base::add_edge(root, v2); }

	debug_graph_t(bool dump_on_exit = true) :
		base("debug_graph"),
		dump_on_exit(dump_on_exit)
#ifdef DEBUG_GRAPH
		, root(base::add_vertex()) // TODO: label root in ctor?
#endif
	{}
	~debug_graph_t()
	{
		// do this before base graph dumps...
#ifdef REMOVE_ORPHANS
		typename base::graph_t without_orphans;
		orphans_removed(without_orphans);
		base::graph = without_orphans;
#endif
	}
};

#endif // BASE_GRAPH_H
