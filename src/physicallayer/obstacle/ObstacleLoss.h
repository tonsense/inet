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

#ifndef __INET_OBSTACLELOSS_H_
#define __INET_OBSTACLELOSS_H_

#include "IObstacleLoss.h"
#include "PhysicalEnvironment.h"

namespace physicallayer
{

class INET_API ObstacleLoss : public cModule, public IObstacleLoss
{
    protected:
        PhysicalEnvironment *environment;

    protected:
        virtual void initialize(int stage);
        virtual double computeObstacleLoss(const PhysicalObject *object, Hz frequency, const Coord transmissionPosition, const Coord receptionPosition) const;

    public:
        ObstacleLoss();

        virtual void printToStream(std::ostream &stream) const;
        virtual double computeObstacleLoss(Hz frequency, const Coord transmissionPosition, const Coord receptionPosition) const;
};

}

#endif