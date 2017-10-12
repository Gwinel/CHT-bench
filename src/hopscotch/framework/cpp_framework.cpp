#include "cpp_framework.h"

//------------------------------------------------------------------------------
// File    : cpp_framework.cpp
// Author  : Ms.Moran Tzafrir
// Written : 13 April 2009
// 
// Multi-Platform C++ framework
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License 
// along with this program; if not, write to the Free Software Foundation
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//------------------------------------------------------------------------------

CMDR::HazardMemory::ScanHazardsThread	CMDR::HazardMemory::_scan_thread;

CMDR::HazardMemory::ThreadHazardsInfo	CMDR::HazardMemory::_thread_ary[HazardMemory::_MAX_THREADS];

const int CMDR::Integer::MIN_VALUE = INT_MIN;
const int CMDR::Integer::MAX_VALUE = INT_MAX;
const int CMDR::Integer::SIZE = 32;

__thread__	CMDR::Thread* CMDR::_g_tls_current_thread = null;
__thread__	CMDR::HazardMemory::ThreadHazardsInfo* CMDR::HazardMemory::_g_tls_thread_hazards = null;

CMDR::HazardMemory::CoarseQueue<CMDR::HazardMemory::Object* volatile*>		CMDR::HazardMemory::_queue;

