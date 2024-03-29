/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2010-2010 OpenCFD Ltd.
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

#include "standardRadiation.H"
#include "volFields.H"
#include "addToRunTimeSelectionTable.H"
#include "zeroGradientFvPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(standardRadiation, 0);

addToRunTimeSelectionTable
(
    filmRadiationModel,
    standardRadiation,
    dictionary
);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

standardRadiation::standardRadiation
(
    const surfaceFilmModel& owner,
    const dictionary& dict
)
:
    filmRadiationModel(typeName, owner, dict),
    QrPrimary_
    (
        IOobject
        (
            //pqr "Qr", // (net radiation) same name as Qr on primary region to enable mapping
            "Qin", // (incident radiation) same name as Qr on primary region to enable mapping
            owner.time().timeName(),
            owner.regionMesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        owner.regionMesh(),
        dimensionedScalar("zero", dimMass/pow3(dimTime), 0.0),
        owner.mappedPushedFieldPatchTypes<scalar>()
    ),
    QrNet_
    (
        IOobject
        (
            "QrNet",
            owner.time().timeName(),
            owner.regionMesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        owner.regionMesh(),
        dimensionedScalar("zero", dimMass/pow3(dimTime), 0.0),
        zeroGradientFvPatchScalarField::typeName
    ),
    /*QrFilm_ is used for diagnostics*/
    QrFilm_
    (
        IOobject
        (
            "QrFilm",
            owner.time().timeName(),
            owner.regionMesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        owner.regionMesh(),
        dimensionedScalar("zero", dimMass/pow3(dimTime), 0.0),
        zeroGradientFvPatchScalarField::typeName
    ),
    delta_(owner.delta()),
    deltaMin_(readScalar(coeffs_.lookup("deltaMin"))),
    beta_(readScalar(coeffs_.lookup("beta"))),
    kappaBar_(readScalar(coeffs_.lookup("kappaBar")))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

standardRadiation::~standardRadiation()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void standardRadiation::correct()
{
    // Transfer Qr from primary region
    QrPrimary_.correctBoundaryConditions();
}


tmp<volScalarField> standardRadiation::Shs()
{
    tmp<volScalarField> tShs
    (
        new volScalarField
        (
            IOobject
            (
                typeName + "::Shs",
                owner().time().timeName(),
                owner().regionMesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            owner().regionMesh(),
            dimensionedScalar("zero", dimMass/pow3(dimTime), 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    scalarField& Shs = tShs();
    /*QrPrimary is radiation from gas phase*/
    const scalarField& QrP = QrPrimary_.internalField();
    const scalarField& delta = delta_.internalField();

    /*Shs is used in film energy equation*/
    /*kvm, for now we assume that water is black to radiation*/
    /*Shs = beta_*(QrP*pos(delta - deltaMin_))*(1.0 - exp(-kappaBar_*delta));*/
    Shs = QrP*pos(delta - deltaMin_); //kvm, for now assume if surface is wet, then radiation is absorbed by film
    QrFilm_.internalField() = Shs;  //kvm, this opens up the film's absorbed radiation to diagnostics
    QrFilm_.correctBoundaryConditions();

    // Update net Qr on local region
    /*QrNet_ is passed on to pyrolysis region (Qr in 0/pyrolysisRegion/Qr) */
    QrNet_.internalField() = QrP - Shs;
    QrNet_.correctBoundaryConditions();

    return tShs;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace Foam

// ************************************************************************* //
