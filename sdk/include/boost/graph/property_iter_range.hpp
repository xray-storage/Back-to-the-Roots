
// (C) Copyright Fran�ois Faure, iMAGIS-GRAVIR / UJF, 2001. Permission
// to copy, use, modify, sell and distribute this software is granted
// provided this copyright notice appears in all copies. This software
// is provided "as is" without express or implied warranty, and with
// no claim as to its suitability for any purpose.

// Revision History:
// 03 May 2001   Jeremy Siek
//      Generalized the property map iterator and moved that
//      part to boost/property_map.hpp. Also modified to
//      differentiate between const/mutable graphs and
//      added a workaround to avoid partial specialization.

// 02 May 2001   Fran�ois Faure
//     Initial version.

#ifndef BOOST_GRAPH_PROPERTY_ITER_RANGE_HPP
#define BOOST_GRAPH_PROPERTY_ITER_RANGE_HPP

#include <boost/property_map_iterator.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/ct_if.hpp>
#include <boost/type_traits/same_traits.hpp>

namespace boost_cryray {

//======================================================================
// graph property iterator range

  template <class Graph, class PropertyTag>
  class graph_property_iter_range {
    typedef typename property_map<Graph, PropertyTag>::type map_type;
    typedef typename property_map<Graph, PropertyTag>::const_type 
      const_map_type;
    typedef typename property_kind<PropertyTag>::type Kind;
    typedef typename ct_if<is_same<Kind, vertex_property_tag>::value,
       typename graph_traits<Graph>::vertex_iterator,
       typename graph_traits<Graph>::edge_iterator>::type iter;
  public:
    typedef typename property_map_iterator_generator<map_type, iter>::type 
      iterator;
    typedef typename property_map_iterator_generator<const_map_type, iter>
      ::type const_iterator;
    typedef std::pair<iterator, iterator> type;
    typedef std::pair<const_iterator, const_iterator> const_type;
  };

  namespace detail {

    template<class Graph,class Tag>
    typename graph_property_iter_range<Graph,Tag>::type
    get_property_iter_range_kind(Graph& graph, const Tag& tag, 
                                 const vertex_property_tag& )
    {
      typedef typename graph_property_iter_range<Graph,Tag>::iterator iter;
      return std::make_pair(iter(vertices(graph).first, get(tag, graph)),
                            iter(vertices(graph).second, get(tag, graph)));
    }

    template<class Graph,class Tag>
    typename graph_property_iter_range<Graph,Tag>::const_type
    get_property_iter_range_kind(const Graph& graph, const Tag& tag, 
                                 const vertex_property_tag& )
    {
      typedef typename graph_property_iter_range<Graph,Tag>
        ::const_iterator iter;
      return std::make_pair(iter(vertices(graph).first, get(tag, graph)),
                            iter(vertices(graph).second, get(tag, graph)));
    }


    template<class Graph,class Tag>
    typename graph_property_iter_range<Graph,Tag>::type
    get_property_iter_range_kind(Graph& graph, const Tag& tag, 
                                 const edge_property_tag& )
    {
      typedef typename graph_property_iter_range<Graph,Tag>::iterator iter;
      return std::make_pair(iter(edges(graph).first, get(tag, graph)),
                            iter(edges(graph).second, get(tag, graph)));
    }

    template<class Graph,class Tag>
    typename graph_property_iter_range<Graph,Tag>::const_type
    get_property_iter_range_kind(const Graph& graph, const Tag& tag, 
                                 const edge_property_tag& )
    {
      typedef typename graph_property_iter_range<Graph,Tag>
        ::const_iterator iter;
      return std::make_pair(iter(edges(graph).first, get(tag, graph)),
                            iter(edges(graph).second, get(tag, graph)));
    }

  } // namespace detail

  //======================================================================
  // get an iterator range of properties

  template<class Graph, class Tag>
  typename graph_property_iter_range<Graph, Tag>::type
  get_property_iter_range(Graph& graph, const Tag& tag)
  {
    typedef typename property_kind<Tag>::type Kind;
    return detail::get_property_iter_range_kind(graph, tag, Kind());
  }

  template<class Graph, class Tag>
  typename graph_property_iter_range<Graph, Tag>::const_type
  get_property_iter_range(const Graph& graph, const Tag& tag)
  {
    typedef typename property_kind<Tag>::type Kind;
    return detail::get_property_iter_range_kind(graph, tag, Kind());
  }

} // namespace boost_cryray


#endif // BOOST_GRAPH_PROPERTY_ITER_RANGE_HPP
