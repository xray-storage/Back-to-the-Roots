
// (C) Copyright Rani Sharoni 2003.
// Permission to copy, use, modify, sell and distribute this software is 
// granted provided this copyright notice appears in all copies. This software 
// is provided "as is" without express or implied warranty, and with no claim 
// as to its suitability for any purpose.
//
// See http://www.boost.org for most recent version including documentation.

#ifndef BOOST_TT_IS_BASE_AND_DERIVED_HPP_INCLUDED
#define BOOST_TT_IS_BASE_AND_DERIVED_HPP_INCLUDED

#include "boost/type_traits/is_class.hpp"
#include "boost/type_traits/is_same.hpp"
#include "boost/type_traits/is_convertible.hpp"
#include "boost/type_traits/detail/ice_and.hpp"
#include "boost/type_traits/remove_cv.hpp"
#include "boost/config.hpp"

// should be the last #include
#include "boost/type_traits/detail/bool_trait_def.hpp"

namespace boost_cryray {

namespace detail {

#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x570)) \
 && !BOOST_WORKAROUND(__SUNPRO_CC , BOOST_TESTED_AT(0x540)) \
 && !BOOST_WORKAROUND(__EDG_VERSION__, <= 238)
                             // The EDG version number is a lower estimate.
                             // It is not currently known which EDG version
                             // exactly fixes the problem.

/*************************************************************************

This version detects ambiguous base classes and private base classes
correctly, and was devised by Rani Sharoni.

Explanation by Terje Sletteb� and Rani Sharoni.

Let's take the multiple base class below as an example, and the following
will also show why there's not a problem with private or ambiguous base
class:

struct B {};
struct B1 : B {};
struct B2 : B {};
struct D : private B1, private B2 {};

is_base_and_derived<B, D>::value;

First, some terminology:

SC  - Standard conversion
UDC - User-defined conversion

A user-defined conversion sequence consists of an SC, followed by an UDC,
followed by another SC. Either SC may be the identity conversion.

When passing the default-constructed Host object to the overloaded check()
functions (initialization 8.5/14/4/3), we have several viable implicit
conversion sequences:

For "static no_type check(B const volatile *, int)" we have the conversion
sequences:

C -> C const (SC - Qualification Adjustment) -> B const volatile* (UDC)
C -> D const volatile* (UDC) -> B1 const volatile* / B2 const volatile* ->
     B const volatile* (SC - Conversion)

For "static yes_type check(D const volatile *, T)" we have the conversion
sequence:

C -> D const volatile* (UDC)

According to 13.3.3.1/4, in context of user-defined conversion only the
standard conversion sequence is considered when selecting the best viable
function, so it only considers up to the user-defined conversion. For the
first function this means choosing between C -> C const and C -> C, and it
chooses the latter, because it's a proper subset (13.3.3.2/3/2) of the
former. Therefore, we have:

C -> D const volatile* (UDC) -> B1 const volatile* / B2 const volatile* ->
     B const volatile* (SC - Conversion)
C -> D const volatile* (UDC)

Here, the principle of the "shortest subsequence" applies again, and it
chooses C -> D const volatile*. This shows that it doesn't even need to
consider the multiple paths to B, or accessibility, as that possibility is
eliminated before it could possibly cause ambiguity or access violation.

If D is not derived from B, it has to choose between C -> C const -> B const
volatile* for the first function, and C -> D const volatile* for the second
function, which are just as good (both requires a UDC, 13.3.3.2), had it not
been for the fact that "static no_type check(B const volatile *, int)" is
not templated, which makes C -> C const -> B const volatile* the best choice
(13.3.3/1/4), resulting in "no".

Also, if Host::operator B const volatile* hadn't been const, the two
conversion sequences for "static no_type check(B const volatile *, int)", in
the case where D is derived from B, would have been ambiguous.

See also
http://groups.google.com/groups?selm=df893da6.0301280859.522081f7%40posting.
google.com and links therein.

*************************************************************************/

template <typename B, typename D>
struct bd_helper
{
    template <typename T>
    static type_traits::yes_type check(D const volatile *, T);
    static type_traits::no_type  check(B const volatile *, int);
};

template<typename B, typename D>
struct is_base_and_derived_impl2
{
    struct Host
    {
        operator B const volatile *() const;
        operator D const volatile *();
    };

    BOOST_STATIC_CONSTANT(bool, value =
        sizeof(bd_helper<B,D>::check(Host(), 0)) == sizeof(type_traits::yes_type));
};

#else

//
// broken version:
//
template<typename B, typename D>
struct is_base_and_derived_impl2
{
    BOOST_STATIC_CONSTANT(bool, value =
        (::boost_cryray::is_convertible<D*,B*>::value));
};

#define BOOST_BROKEN_IS_BASE_AND_DERIVED

#endif

template <typename B, typename D>
struct is_base_and_derived_impl3
{
    BOOST_STATIC_CONSTANT(bool, value = false);
};

template <bool ic1, bool ic2, bool iss>
struct is_base_and_derived_select
{
   template <class T, class U>
   struct rebind
   {
      typedef is_base_and_derived_impl3<T,U> type;
   };
};

template <>
struct is_base_and_derived_select<true,true,false>
{
   template <class T, class U>
   struct rebind
   {
      typedef is_base_and_derived_impl2<T,U> type;
   };
};

template <typename B, typename D>
struct is_base_and_derived_impl
{
    typedef typename remove_cv<B>::type ncvB;
    typedef typename remove_cv<D>::type ncvD;

    typedef is_base_and_derived_select<
       ::boost_cryray::is_class<B>::value,
       ::boost_cryray::is_class<D>::value,
       ::boost_cryray::is_same<B,D>::value> selector;
    typedef typename selector::template rebind<ncvB,ncvD> binder;
    typedef typename binder::type bound_type;

    BOOST_STATIC_CONSTANT(bool, value = bound_type::value);
};

} // namespace detail

BOOST_TT_AUX_BOOL_TRAIT_DEF2(
      is_base_and_derived
    , Base
    , Derived
    , (::boost_cryray::detail::is_base_and_derived_impl<Base,Derived>::value)
    )

#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_and_derived,Base&,Derived,false)
BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_and_derived,Base,Derived&,false)
BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_and_derived,Base&,Derived&,false)
#endif

} // namespace boost_cryray

#include "boost/type_traits/detail/bool_trait_undef.hpp"

#endif // BOOST_TT_IS_BASE_AND_DERIVED_HPP_INCLUDED
