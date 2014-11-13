#ifndef TYPES_H
#define TYPES_H

#include "patch.h"
#include "ca_basics.h"

using namespace sca;

constexpr const bool extended_format = true;

//using char_traits = traits<char, short, char>;
using char_traits = coord_traits<int64_t>;
using char_cell_traits = cell_traits<int64_t>;

#define USE_LABELS

class types
{
public:
	using conf_t = ca::_conf_t<char_cell_traits>;
	using cell_t = typename char_cell_traits::cell_t;
	using u_coord_t = typename char_traits::u_coord_t;
	using point = _point<char_traits>;
	using point_itr = _point_itr<char_traits>;
	using grid_t = _grid_t<char_traits, char_cell_traits>;
	using dimension = _dimension<char_traits>;
	using rect = _rect<char_traits>;
	using bounding_box = _bounding_box<char_traits>;
	using patch_t = _patch_t<extended_format, char_traits, char_cell_traits>;
	using backed_up_grid = _backed_up_grid<extended_format, char_traits, char_cell_traits>;
	using n_t = ca::_n_t<char_traits, std::vector<point>>;
};

#endif // TYPES_H
