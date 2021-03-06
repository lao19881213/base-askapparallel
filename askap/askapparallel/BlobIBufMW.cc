/// @file BlobIBufMW.cc
///
/// @copyright (c) 2011 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "BlobIBufMW.h"

// System includes
#include <vector>
#include <algorithm>

// ASKAPSoft includes
#include "askap/askap/AskapError.h"
#include "Common/LofarTypes.h"

// Local package includes
#include "askap/askapparallel/AskapParallel.h"

using namespace askap::askapparallel;

// Constructor.
BlobIBufMW::BlobIBufMW(AskapParallel& comms, int rank)
    : itsComms(comms) , itsSrcRank(rank), itsDataBegin(itsBuffer.begin())
{
    ASKAPCHECK(itsComms.isParallel(), 
            "This class cannot be used in non parallel applications");
}

// Destructor.
BlobIBufMW::~BlobIBufMW()
{
}

// Get the requested nr of bytes.
LOFAR::uint64 BlobIBufMW::get(void* buffer, LOFAR::uint64 nbytes)
{
    // 1: If itsBuff doesn't have sufficient data to fulfill the request
    // then get enough data first
    LOFAR::uint64 size;
    while ((itsBuffer.end() - itsDataBegin) < long(nbytes)) {
        // If there is space at the front of the buffer, move the data
        // to use the free space.
        if (itsDataBegin != itsBuffer.begin()) {
            itsBuffer.erase(itsBuffer.begin(), itsDataBegin);
            // itsDataBegin is reset later, after resize is called.
            // It must be done then because the iterator can be
            // invalidated by a call to resize.
        }

        receive(&size, sizeof(LOFAR::uint64));
        ASKAPCHECK(size > 0, "Message of size zero is invalid");
        const size_t oldSize = itsBuffer.size();
        itsBuffer.resize(itsBuffer.size() + size);
        itsDataBegin = itsBuffer.begin();
        receive(&itsBuffer[oldSize], size);
    }

    // 2: Now enough data exists to fulfill the request action it.
    ASKAPCHECK((itsBuffer.end() - itsDataBegin) >= long(nbytes),
            "Buffer doesn't have sufficient data to fulfill request");
    std::copy(itsDataBegin, itsDataBegin + nbytes,
            reinterpret_cast<char*>(buffer));
    itsDataBegin += nbytes;

    return nbytes;
}

// Get the position in the stream.
// -1 is returned if the stream is not seekable.
LOFAR::int64 BlobIBufMW::tellPos() const
{
    return -1;
}

// Set the position in the stream.
// It returns the new position which is -1 if the stream is not seekable.
LOFAR::int64 BlobIBufMW::setPos(LOFAR::int64 pos)
{
    return -1;
}

void BlobIBufMW::receive(void* buffer, size_t nbytes)
{
    itsComms.receive(buffer, nbytes, itsSrcRank);
}
