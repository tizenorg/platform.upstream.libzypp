/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/XMLMessageImpl.cc
 *
*/

#include "zypp/target/store/xml/XMLMessageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLMessageImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    XMLMessageImpl::XMLMessageImpl()
    {}
    /** Dtor */
    XMLMessageImpl::~XMLMessageImpl()
    {}

    std::string XMLMessageImpl::type() const {
      return _type;
    }

    std::string XMLMessageImpl::text() const {
      return _text;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
