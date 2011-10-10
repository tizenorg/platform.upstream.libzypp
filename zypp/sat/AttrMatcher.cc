/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/AttrMatcher.cc
 *
*/
extern "C"
{
#include <solv/repo.h>
}

#include <iostream>
#include <sstream>
#include <boost/mpl/int.hpp>

#include "zypp/base/LogTools.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"

#include "zypp/sat/AttrMatcher.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Match
  //
  ///////////////////////////////////////////////////////////////////

  const int Match::_modemask = SEARCH_STRINGMASK;
  const int Match::_flagmask = ~_modemask;

  // option flags
  const Match Match::NOCASE		(SEARCH_NOCASE);
  const Match Match::NO_STORAGE_SOLVABLE(SEARCH_NO_STORAGE_SOLVABLE);
  const Match Match::SUB		(SEARCH_SUB);
  const Match Match::ARRAYSENTINEL	(SEARCH_ARRAYSENTINEL);
  const Match Match::SKIP_KIND		(SEARCH_SKIP_KIND);
  const Match Match::FILES		(SEARCH_FILES);

  Match::Mode Match::mode() const
  {
    switch ( modeval() )
    {
      case 0:			return NOTHING;		break;
      case SEARCH_STRING:	return STRING;		break;
      case SEARCH_STRINGSTART:	return STRINGSTART;	break;
      case SEARCH_STRINGEND:	return STRINGEND;	break;
      case SEARCH_SUBSTRING:	return SUBSTRING;	break;
      case SEARCH_GLOB:		return GLOB;		break;
      case SEARCH_REGEX:	return REGEX;		break;
    }
    return OTHER;
  }

  int Match::modeval( Mode mode_r )
  {
    switch ( mode_r )
    {
      case NOTHING:	return 0;			break;
      case STRING:	return SEARCH_STRING;		break;
      case STRINGSTART:	return SEARCH_STRINGSTART;	break;
      case STRINGEND:	return SEARCH_STRINGEND;	break;
      case SUBSTRING:	return SEARCH_SUBSTRING;	break;
      case GLOB:	return SEARCH_GLOB;		break;
      case REGEX:	return SEARCH_REGEX;		break;
      case OTHER:	return SEARCH_STRINGMASK;	break;
    }
    return SEARCH_STRINGMASK;
  }

  std::string Match::asString() const
  { std::ostringstream str; str << *this; return str.str(); }

  std::ostream & operator<<( std::ostream & str, Match::Mode obj )
  {
    switch ( obj )
    {
#define OUTS(V) case Match::V: return str << #V; break
      OUTS( NOTHING );
      OUTS( STRING );
      OUTS( STRINGSTART );
      OUTS( STRINGEND );
      OUTS( SUBSTRING );
      OUTS( GLOB );
      OUTS( REGEX );
      OUTS( OTHER );
#undef OUTS
    }
    return str << "Match::Mode::UNKNOWN";
  }

  std::ostream & operator<<( std::ostream & str, const Match & obj )
  {
    if ( ! obj )
      return str << "NOTHING";

    const char * sep = "|";
    Match::Mode mode( obj.mode() );
    switch ( mode )
    {
      case Match::NOTHING:
        sep = 0; // suppress 'NOTHING|'
        break;
      case Match::OTHER:
        str << mode<<"("<<obj.modeval()<<")"; // check whether libsolv has introduced new modes!
        break;
      default:
        str << mode;
        break;
    }

    int val = obj.flagval();
    if ( val )
    {
#define OUTS(V) if ( val & Match::V.get() ) { val &= ~Match::V.get(); if ( sep ) str << sep; else sep = "|"; str << #V; }
      OUTS( NOCASE );
      OUTS( NO_STORAGE_SOLVABLE );
      OUTS( SUB );
      OUTS( ARRAYSENTINEL );
      OUTS( SKIP_KIND );
      OUTS( FILES );
#undef OUTS
      if ( val )
      {
        if ( sep ) str << sep;
        str << zypp::str::hexstring( val ); // check whether libsolv has introduced new flags.
      }
    }
    return str;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : MatchException
  //
  ///////////////////////////////////////////////////////////////////

  MatchUnknownModeException::MatchUnknownModeException( const Match & mode_r, const std::string & msg_r )
  : MatchException( msg_r.empty() ? str::form(_("Unknown match mode '%s'"), mode_r.asString().c_str() )
                                  : str::form(_("Unknown match mode '%s' for pattern '%s'"), mode_r.asString().c_str(), msg_r.c_str() ) )
  {}

  MatchInvalidRegexException::MatchInvalidRegexException( const std::string & regex_r, int regcomp_r )
  : MatchException( regcomp_r ? str::form(_("Invalid regular expression '%s': regcomp returned %d"), regex_r.c_str(), regcomp_r )
                              : str::form(_("Invalid regular expression '%s'"), regex_r.c_str() ) )
  {}

  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : AttrMatcher::Impl
    //
    /** AttrMatcher implementation.
     *
     * Take care to release any allocated regex by calling
     * \c ::datamatcher_free.
     */
    struct AttrMatcher::Impl
    {
      Impl()
      {}

      Impl( const std::string & search_r, const Match & flags_r )
        : _search( search_r )
        , _flags( flags_r )
      {}

      ~Impl()
      { invalidate(); }

      /** Compile the pattern. */
      void compile() const
      {
        if ( !_matcher )
        {
          if ( _flags.mode() == Match::OTHER )
            ZYPP_THROW( MatchUnknownModeException( _flags, _search ) );

          _matcher.reset( new ::_Datamatcher );
          int res = ::datamatcher_init( _matcher.get(), _search.c_str(), _flags.get() );
          if ( res )
          {
            _matcher.reset();
            ZYPP_THROW( MatchInvalidRegexException( _search, res ) );
          }
        }
      }

      /** Whether the pattern is already compiled. */
      bool isCompiled() const
      { return _matcher; }

      /** Return whether string matches. */
      bool doMatch( const char * string_r ) const
      {
        compile(); // nop if already compiled.

        if ( ! string_r )
          return false; // NULL never matches
        return ::datamatcher_match( _matcher.get(), string_r );
      }

      /** The current searchstring. */
      const std::string & searchstring() const
      { return _search; }

      /** Set a new searchstring. */
      void setSearchstring( const std::string & string_r )
      { invalidate(); _search = string_r; }

      /** The current search flags. */
      const Match & flags() const
      { return _flags; }

      /** Set new search flags. */
      void setFlags( const Match & flags_r )
      { invalidate(); _flags = flags_r; }

      private:
        /** Has to be called if _search or _flags change. */
        void invalidate()
        {
          if ( _matcher )
            ::datamatcher_free( _matcher.get() );
          _matcher.reset();
        }

      private:
        std::string _search;
        Match       _flags;
        mutable scoped_ptr< ::_Datamatcher> _matcher;

      private:
        friend Impl * rwcowClone<Impl>( const Impl * rhs );
        /** clone for RWCOW_pointer */
        Impl * clone() const
        { return new Impl( _search, _flags ); }
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates AttrMatcher::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const AttrMatcher::Impl & obj )
    {
      return str << "\"" << obj.searchstring() << "\"{" << obj.flags() << "}";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : AttrMatcher
    //
    ///////////////////////////////////////////////////////////////////

    AttrMatcher::AttrMatcher()
    : _pimpl( new Impl )
    {}

    AttrMatcher::AttrMatcher( const std::string & search_r )
    : _pimpl( new Impl( search_r, Match::STRING ) )
    {}

    AttrMatcher::AttrMatcher( const std::string & search_r, const Match & flags_r )
    : _pimpl( new Impl( search_r, flags_r ) )
    {}

    AttrMatcher::AttrMatcher( const std::string & search_r, const Match::Mode & flags_r )
    : _pimpl( new Impl( search_r, flags_r ) )
    {}

    AttrMatcher::AttrMatcher( const std::string & search_r, int flags_r )
    : _pimpl( new Impl( search_r, Match(flags_r) ) )
    {}

    void AttrMatcher::compile() const
    { return _pimpl->compile(); }

    bool AttrMatcher::isCompiled() const
    { return _pimpl->isCompiled(); }

    bool AttrMatcher::doMatch( const char * string_r ) const
    { return _pimpl->doMatch( string_r ); }

    const std::string & AttrMatcher::searchstring() const
    { return _pimpl->searchstring(); }

    void AttrMatcher::setSearchstring( const std::string & string_r )
    { _pimpl->setSearchstring( string_r ); }

    void AttrMatcher::setSearchstring( const std::string & string_r, const Match & flags_r )
    {
      _pimpl->setSearchstring( string_r );
      _pimpl->setFlags( flags_r );
    }

    const Match & AttrMatcher::flags() const
    { return _pimpl->flags(); }

    void AttrMatcher::setFlags( const Match & flags_r )
    { _pimpl->setFlags( flags_r ); }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const AttrMatcher & obj )
    {
      return str << *obj._pimpl;
    }

    bool operator==( const AttrMatcher & lhs, const AttrMatcher & rhs )
    {
      return ( lhs.flags() == rhs.flags()
               && lhs.searchstring() == rhs.searchstring() );
    }

    bool operator<( const AttrMatcher & lhs, const AttrMatcher & rhs )
    {
      if ( lhs.flags().get() != rhs.flags().get() )
        return ( lhs.flags().get() < rhs.flags().get() );

      return ( lhs.searchstring() < rhs.searchstring() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
