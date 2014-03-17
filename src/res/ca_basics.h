#ifndef CA_BASICS_H
#define CA_BASICS_H

#include "general.h"

/**
 * @brief This class holds input and output values of a
 * local transition function.
 *
 * Where these values are depends on a neighborhood class,
 * which s not a part of this struct.
 */
class transition_function
{
	// group parameters
	const unsigned neighbour_size; //! size of input_vals
	// differing parameters
	std::vector<int> input_vals; //!< function input values
	std::vector<bool> input_set; //!< is the value a dontcare?
	int input_count = 0; //!< number of set bits in input_set
	int output; //! function output value

public:
	transition_function(const unsigned _neighbour_size)
	:
		neighbour_size(_neighbour_size),
		input_vals(neighbour_size),
		input_set(neighbour_size, false)
	{
	}

	transition_function operator=(
		const transition_function& other)
	{
		assert(neighbour_size == other.neighbour_size);
		input_vals = other.input_vals;
		input_set = other.input_set;
		input_count = other.input_count;
		output = other.output;
		return *this;
	}

	transition_function(
		int _neighbour_size,
		const std::vector<int>& input_grid,
		int output_val
		) : // TODO: 2ctors + reuse
		neighbour_size(_neighbour_size),
		input_vals(neighbour_size),
		input_set(neighbour_size, false),
		output(output_val)
	{
	}

	void set_neighbour(int neighbour_id, int val)
	{
		input_vals[neighbour_id] = val;
		if(!input_set[neighbour_id])
		{
			input_set[neighbour_id] = true;
			++input_count;
		}
		else assert(false);
	}
	int get_output() const { return output; }
	bool input(int neighbour_id, int* result) {
		bool is_set = input_set[neighbour_id];
		if(is_set) *result = input_vals[neighbour_id];
		return is_set;
	}

	// compares lexicographical, returns results for operator<
	// a dontcare in front counts as a smaller value
	friend bool _compare_by_input(const transition_function& lhs,
		const transition_function& rhs)
	{
		// TODO: assert same neighbour size, grid size?
		for(unsigned i = 0; i<lhs.neighbour_size; ++i)
		{
			if(lhs.input_set[i] || rhs.input_set[i])
			{
				// if one of tem is not set,
				// the one which is not set is smaller
				if(!(lhs.input_set[i]&&rhs.input_set[i]))
				 return (int)lhs.input_set[i]
					> (int)rhs.input_set[i];
				else if(lhs.input_vals[i]
					!=rhs.input_vals[i])
				 return (lhs.input_vals[i]
					<rhs.input_vals[i]);
			}
		}
		return false; // if both are equal
	}

	bool operator<(const transition_function& rhs) const
	{
		return (output == rhs.output)
			? _compare_by_input(*this, rhs)
			: (output < rhs.output);
	}

	bool operator==(const transition_function& rhs) const
	{
		if(input_count != rhs.input_count)
		 return false; // shortening
		for(unsigned i = 0; i<neighbour_size; ++i)
		{
			if(input_set[i]&&rhs.input_set[i])
			{
				if(input_vals[i]!=rhs.input_vals[i])
				 return false;
			}
			else if(input_set[i]!=input_set[i])
			  return false; // one is not set
		}
		return true;
	}

	bool operator!=(const transition_function& rhs) const
	{
		return ! operator==(rhs);
	}
};

inline bool compare_by_input(const transition_function& lhs,
		const transition_function& rhs)
{
	return _compare_by_input(lhs, rhs);
}

class neighbourhood
{
	//! neighbour positions, relative to center cell
	std::vector<point> neighbours;
	//! positive offset of center cell
	//! this cell is obviously needed
//	point center_cell = point(-1, -1);
	dimension dim; 	//! < dimension of neighbours

	void init(const std::vector<int>& in_grid,
		const dimension& in_dim)
	{
		point center_cell(-1, -1);
		for(unsigned y=0, id=0; y<in_dim.height; ++y)
		for(unsigned x=0; x<in_dim.width; ++x, ++id) // TODO: remove id
		{
			int elem = in_grid[id];
			switch(elem)
			{
			case 0:
				//printf("%d\n",center_cell.x);
				assert(center_cell.x < 0);
				center_cell.set(x, y);
				break;
			case 1:
			//	printf("%d\n",center_cell.x);
				assert(center_cell.x < 0);
				center_cell.set(x, y);
			case 2:
				neighbours.push_back(point(x,y));
				break;
			default: break;
			}
		}

		// make it all relative to center_cell
		for(point& p : neighbours)
		 p -= center_cell; // TODO: make this in case 2?
	}

	point idx(int idx, int symm)
	{
		point p = operator[](idx);
		const int rot = symm & 3;
		const int refl = symm >> 2;

		static const matrix rot_mat(0, -1, 1, 0);
		static const matrix rot_mats[] =
		 { matrix::id, rot_mat, rot_mat^2, rot_mat^3 };
		static const matrix mirror_mats[] =
		 { matrix::id, matrix(1, 0, 0, -1) };

		const point new_point
		= rot_mats[rot] * (mirror_mats[refl] * p);
		return new_point;
	}


	void add_single_tf(
		transition_function* tfs,
		const std::vector<int>& input_grid,
		int output_val,
		int symm)
	{
		transition_function tf(size(), input_grid, output_val);
		for(unsigned i = 0; i < size(); ++i)
		 tf.set_neighbour(i, input_grid[dim.coords_to_id(idx(i, symm)/*+center_cell*/)]);
		*tfs = tf; // TODO: redundant
	}

public:
	point operator[](unsigned i) const { return neighbours[i]; }
	unsigned size() const { return neighbours.size(); }

	void add_transition_functions(
		std::vector<transition_function>& tf_vector,
		const std::vector<int>& input_grid,
		int output_val)
	{
		static transition_function tfs[8]
		 = {size(), size(), size(), size(),
			size(), size(), size(), size()};

		// TODO: unroll?
		for(int i = 0; i < 8; ++i)
		 add_single_tf(tfs+i, input_grid, output_val, i);

		std::sort(tfs, tfs+8);
		for(int i = 0; i < 8; ++i)
		 if((i==0) || (tfs[i-1] != tfs[i]))
		  tf_vector.push_back(tfs[i]);
	}

	/**
	 * @brief Reads neighborhood from grid.
	 *
	 * Cell values:
	 *  - 0 center cell which is *no* part of the nh.
	 *  - 1 center cell which *is* part of the nh.
	 *  - 2 nh cell
	 *  - 3 other cell (TODO: change: 2 grids of 0,1)
	 *
	 * @param in_grid
	 * @param in_dim
	 */
	neighbourhood(const std::vector<int>& in_grid,
		const dimension& in_dim)
	{
		// TODO: parameter in_dim is useless?
		init(in_grid, in_dim);
	}

	neighbourhood(FILE* fp)
	{
		std::vector<int> in_grid;
		read_grid(stdin, &in_grid, &dim, 0);
		init(in_grid, dim);
	}

	/*neighbourhood(const dimension_container& _dim, point _center_cell = {0,0})
		: center_cell(_center_cell),
		dim({_dim.h, _dim.w})
	{
		neighbours.reserve(dim.area());
		for( const point& p : _dim )
		 neighbours.push_back(p);
	}*/

	//! assumes that no borders exist
	neighbourhood(const dimension& _dim, point _center_cell = {0,0})
		: //center_cell(_center_cell),
		dim(_dim)
	{
		neighbours.reserve(_dim.area());
		dimension_container cont(_dim.height, _dim.width, 0);
		for( const point& p : cont )
		{
			std::cout << "creating n: " << p << std::endl;
			neighbours.push_back(p - _center_cell);
		}
	}

/*	neighbourhood(const std::vector<point>& _points, point _center_cell)
		: center_cell(_center_cell)
	{
		neighbours.reserve(_points.size());
		bounding_box bb;
		for( const point& p : _points )
		{
			point new_p = p - _center_cell;
			neighbours.push_back(new_p);
			bb.add_point(new_p);
		}
		dim = bb.dim();
	}*/
};

#endif // CA_BASICS_H
