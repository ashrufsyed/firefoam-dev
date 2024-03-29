/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2009-2010 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "phaseChangeModel.H"
#include "stdio.h"

#define DEBUG(x) std::cout << "["<< __FILE__ << ":" << __LINE__ << "] "<< #x " = " << x << std::endl;
#define TRACE(s) std::cout << "["<< __FILE__ << ":" << __LINE__ << "] "<< #s << std::endl; s;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(phaseChangeModel, 0);
defineRunTimeSelectionTable(phaseChangeModel, dictionary);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

phaseChangeModel::phaseChangeModel
(
    const surfaceFilmModel& owner
)
:
    subModelBase(owner),
    latestMassPC_(0.0),
    totalMassPC_(0.0)
{}


phaseChangeModel::phaseChangeModel
(
    const word& type,
    const surfaceFilmModel& owner,
    const dictionary& dict
)
:
    subModelBase(type, owner, dict),
    latestMassPC_(0.0),
    totalMassPC_(0.0)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

phaseChangeModel::~phaseChangeModel()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void phaseChangeModel::correct
(
    const scalar dt,
    scalarField& availableMass,
    volScalarField& dMass,
    volScalarField& dEnergy
)
{
    correctModel
    (
        dt,
        availableMass,
        dMass,
        dEnergy
    );

    latestMassPC_ = sum(dMass.internalField());
    totalMassPC_ += latestMassPC_; 

    availableMass -= dMass;
    dMass.correctBoundaryConditions();
}


void phaseChangeModel::info(Ostream& os) const
{
    const scalar massPCRate =
        returnReduce(latestMassPC_, sumOp<scalar>())
       /owner_.time().deltaTValue();

    os  << indent << "mass phase change  = "
        << returnReduce(totalMassPC_, sumOp<scalar>()) << nl
        << indent << "vapourisation rate = " << massPCRate << nl;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // end namespace surfaceFilmModels
} // end namespace regionModels
} // end namespace Foam

// ************************************************************************* //
