/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2009-2010 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "pyrolysisModel.H"
#include "fvMesh.H"
#include "directMappedFieldFvPatchField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace pyrolysisModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(pyrolysisModel, 0);
defineRunTimeSelectionTable(pyrolysisModel, mesh);

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

void pyrolysisModel::constructMeshObjects()
{
    // construct filmDelta field if coupled to film model
    if (filmCoupled_)
    {
        filmDeltaPtr_.reset
        (
            new volScalarField
            (
                IOobject
                (
                    "filmDelta",
                    time_.timeName(),
                    regionMesh(),
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE
                ),
                regionMesh()
            )
        );

        filmTemperaturePtr_.reset
        (
            new volScalarField
            (
                IOobject
                (
                    "filmTemperature",
                    time_.timeName(),
                    regionMesh(),
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE
                ),
                regionMesh()
            )
        );

        filmConvPtr_.reset
        (
            new volScalarField
            (
                IOobject
                (
                    "filmConv",
                    time_.timeName(),
                    regionMesh(),
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE
                ),
                regionMesh()
            )
        );

        const volScalarField& filmDelta = filmDeltaPtr_();
        const volScalarField& filmTemperature = filmTemperaturePtr_();
        const volScalarField& filmConv = filmConvPtr_();

        bool foundCoupledPatch = false;
        forAll(filmDelta.boundaryField(), patchI)
        {
            const fvPatchField<scalar>& fvp = filmDelta.boundaryField()[patchI];
            if (isA<directMappedFieldFvPatchField<scalar> >(fvp))
            {
                foundCoupledPatch = true;
                break;
            }
        }

        if (!foundCoupledPatch)
        {
            WarningIn("void pyrolysisModels::constructMeshObjects()")
                << "filmCoupled flag set to true, but no "
                << directMappedFieldFvPatchField<scalar>::typeName
                << " patches found on " << filmDelta.name() << " field"
                << endl;
        }

        foundCoupledPatch = false;
        forAll(filmTemperature.boundaryField(), patchI)
        {
            const fvPatchField<scalar>& fvp = filmTemperature.boundaryField()[patchI];
            if (isA<directMappedFieldFvPatchField<scalar> >(fvp))
            {
                foundCoupledPatch = true;
                break;
            }
        }

        if (!foundCoupledPatch)
        {
            WarningIn("void pyrolysisModels::constructMeshObjects()")
                << "filmCoupled flag set to true, but no "
                << directMappedFieldFvPatchField<scalar>::typeName
                << " patches found on " << filmTemperature.name() << " field"
                << endl;
        }

        foundCoupledPatch = false;
        forAll(filmConv.boundaryField(), patchI)
        {
            const fvPatchField<scalar>& fvp = filmConv.boundaryField()[patchI];
            if (isA<directMappedFieldFvPatchField<scalar> >(fvp))
            {
                foundCoupledPatch = true;
                break;
            }
        }

        if (!foundCoupledPatch)
        {
            WarningIn("void pyrolysisModels::constructMeshObjects()")
                << "filmCoupled flag set to true, but no "
                << directMappedFieldFvPatchField<scalar>::typeName
                << " patches found on " << filmConv.name() << " field"
                << endl;
        }
    }
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool pyrolysisModel::read()
{
    if (regionModel1D::read())
    {
        filmCoupled_ = readBool(coeffs_.lookup("filmCoupled"));
        reactionDeltaMin_ =
            coeffs_.lookupOrDefault<scalar>("reactionDeltaMin", 0.0);

        return true;
    }
    else
    {
        return false;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

pyrolysisModel::pyrolysisModel(const fvMesh& mesh)
:
    regionModel1D(mesh),
    filmCoupled_(false),
    filmDeltaPtr_(NULL),
    filmTemperaturePtr_(NULL),
    filmConvPtr_(NULL),
    reactionDeltaMin_(0.0)
{}


pyrolysisModel::pyrolysisModel(const word& modelType, const fvMesh& mesh)
:
    regionModel1D(mesh, "pyrolysis", modelType),
    filmCoupled_(false),
    filmDeltaPtr_(NULL),
    filmTemperaturePtr_(NULL),
    filmConvPtr_(NULL),
    reactionDeltaMin_(0.0)
{
    if (active_)
    {
        read();
        constructMeshObjects();
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

pyrolysisModel::~pyrolysisModel()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

scalar pyrolysisModel::addMassSources
(
    const label patchI,
    const label faceI
)
{
    return 0.0;
}


void pyrolysisModel::preEvolveRegion()
{
    if (filmCoupled_)
    {
        filmDeltaPtr_->correctBoundaryConditions();
        filmTemperaturePtr_->correctBoundaryConditions();
        filmConvPtr_->correctBoundaryConditions();
        if(time_.outputTime()){
            filmTemperaturePtr_->write();
            filmConvPtr_->write();
        }
    }
}


scalar pyrolysisModel::solidRegionDiffNo() const
{
    return VSMALL;
}


scalar pyrolysisModel::maxDiff() const
{
    return GREAT;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace Foam

// ************************************************************************* //
