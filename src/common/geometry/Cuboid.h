//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_CUBOID_H
#define __INET_CUBOID_H

#include "Shape.h"
#include "Prism.h"
#include "Polygon.h"

namespace inet {

/**
 * This class represents a 3 dimensional shape with 6 pairwise parallel faces.
 */
class INET_API Cuboid : public Prism
{
  protected:
    Coord size;

  public:
    Cuboid(const Coord& size);

    const Coord& getSize() const { return size; }
    void setSize(const Coord& size) { this->size = size; }
    virtual Coord computeSize() const { return size; }
};

} // namespace inet

#endif // ifndef __INET_CUBOID_H
