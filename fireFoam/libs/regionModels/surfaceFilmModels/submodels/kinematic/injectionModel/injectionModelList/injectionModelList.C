/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2011 OpenCFD Ltd.
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

#include "injectionModelList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmModels
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

injectionModelList::injectionModelList(const surfaceFilmModel& owner)
:
    PtrList<injectionModel>(),
    owner_(owner),
    dict_(dictionary::null),
    injectedMassTotal_(0.0)
{}


injectionModelList::injectionModelList
(
    const surfaceFilmModel& owner,
    const dictionary& dict
)
:
    PtrList<injectionModel>(),
    owner_(owner),
    dict_(dict),
    injectedMassTotal_(0.0)
{
    const wordList activeModels(dict.lookup("injectionModels"));

    wordHashSet models;
    forAll(activeModels, i)
    {
        models.insert(activeModels[i]);
    }

    Info<< "    Selecting film injection models" << endl;
    if (models.size() > 0)
    {
        this->setSize(models.size());

        label i = 0;
        forAllConstIter(wordHashSet, models, iter)
        {
            const word& model = iter.key();
            set
            (
                i,
                injectionModel::New(owner, dict, model)
            );
            i++;
        }
    }
    else
    {
        Info<< "        none" << endl;
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

injectionModelList::~injectionModelList()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void injectionModelList::correct
(
    scalarField& availableMass,
    volScalarField& massToInject,
    volScalarField& diameterToInject
)
{
    // Correct models that accumulate mass and diameter transfers
    forAll(*this, i)
    {
        injectionModel& im = operator[](i);
        im.correct(availableMass, massToInject, diameterToInject);
    }

    injectedMassTotal_ += sum(massToInject.internalField());


    // Push values to boundaries ready for transfer to the primary region
    massToInject.correctBoundaryConditions();
    diameterToInject.correctBoundaryConditions();
}


void injectionModelList::info(Ostream& os) const
{
    os  << indent << "injected mass      = "
        << returnReduce<scalar>(injectedMassTotal_, sumOp<scalar>()) << nl;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace Foam

// ************************************************************************* //
