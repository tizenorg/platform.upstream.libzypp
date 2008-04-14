/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* SolverQueueItem.cc
 *
 * Copyright (C) 2008 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "zypp/base/Logger.h"
#include "zypp/sat/SolverQueueItemInstall.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

IMPL_PTR_TYPE(SolverQueueItemInstall);

//---------------------------------------------------------------------------

std::ostream &
SolverQueueItemInstall::dumpOn( std::ostream & os ) const
{
    os << "[" << (_soft?"Soft":"") << "Install: ";
    os << _name;

    return os;
}

//---------------------------------------------------------------------------

SolverQueueItemInstall::SolverQueueItemInstall (const ResPool & pool, std::string name, bool soft)
    : SolverQueueItem (QUEUE_ITEM_TYPE_INSTALL, pool)
    , _name (name)
    , _soft (soft)
{
}


SolverQueueItemInstall::~SolverQueueItemInstall()
{
}

//---------------------------------------------------------------------------

SolverQueueItem_Ptr
SolverQueueItemInstall::copy (void) const
{
    SolverQueueItemInstall_Ptr new_install = new SolverQueueItemInstall (pool(), _name);
    new_install->SolverQueueItem::copy(this);

    new_install->_soft = _soft;
    return new_install;
}


//---------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
