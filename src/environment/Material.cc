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

#include "Material.h"
#include "INETMath.h"

namespace inet {

// TODO: check values, add frequency dependence
const Material Material::vacuum("vacuum", Ohmm(sNaN), 1, 1);
const Material Material::air("air", Ohmm(sNaN), 1.00058986, 1.00000037);
const Material Material::copper("copper", Ohmm(1.68), sNaN, sNaN);
const Material Material::aluminium("aluminium", Ohmm(2.65), sNaN, sNaN);
const Material Material::wood("wood", Ohmm(1E+15), 5, 1.00000043);
const Material Material::brick("brick", Ohmm(3E+3), 4.5, 1);
const Material Material::concrete("concrete", Ohmm(1E+2), 4.5, 1);
const Material Material::glass("glass", Ohmm(1E+12), 7, 1);

Material::Material(const char *name, Ohmm resistivity, double relativePermittivity, double relativePermeability) :
    cNamedObject(name),
    resistivity(resistivity),
    relativePermittivity(relativePermittivity),
    relativePermeability(relativePermeability)
{
}

Material::~Material()
{
}

double Material::getDielectricLossTangent(Hz frequency) const
{
    return unit(1.0 / (2 * M_PI * frequency * resistivity * relativePermittivity * e0)).get();
}

double Material::getRefractiveIndex() const
{
    return std::sqrt(relativePermittivity * relativePermeability);
}

mps Material::getPropagationSpeed() const
{
    return mps(SPEED_OF_LIGHT) / getRefractiveIndex();
}

} // namespace inet

