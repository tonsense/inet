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

#ifndef __INET_MATERIAL_H
#define __INET_MATERIAL_H

#include "INETDefs.h"
#include "Units.h"

namespace inet {

using namespace units::values;
using namespace units::constants;

class INET_API Material : public cNamedObject
{
  public:
    static const Material vacuum;
    static const Material air;
    static const Material copper;
    static const Material aluminium;
    static const Material wood;
    static const Material brick;
    static const Material concrete;
    static const Material glass;

  protected:
    const Ohmm resistivity;
    const double relativePermittivity;
    const double relativePermeability;

  public:
    Material(const char *name, Ohmm resistivity, double relativePermittivity, double relativePermeability);
    virtual ~Material();

    virtual Ohmm getResistivity() const { return resistivity; }
    virtual double getRelativePermittivity() const { return relativePermittivity; }
    virtual double getRelativePermeability() const { return relativePermeability; }
    virtual double getDielectricLossTangent(Hz frequency) const;
    virtual double getRefractiveIndex() const;
    virtual mps getPropagationSpeed() const;
};

} // namespace inet

#endif // ifndef __INET_MATERIAL_H

