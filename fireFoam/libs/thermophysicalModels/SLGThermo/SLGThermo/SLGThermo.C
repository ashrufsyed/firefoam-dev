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

#include "SLGThermo.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(SLGThermo, 0);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::SLGThermo::SLGThermo(const fvMesh& mesh, basicThermo& thermo)
:
    MeshObject<fvMesh, SLGThermo>(mesh),
    thermo_(thermo),
    carrier_(NULL),
    liquids_(NULL),
    solids_(NULL)
{
    Info<< "Creating component thermo properties:" << endl;

    if (isA<basicMultiComponentMixtureNew>(thermo))
    {
        basicMultiComponentMixtureNew& mcThermo =
            dynamic_cast<basicMultiComponentMixtureNew&>(thermo);
        carrier_ = &mcThermo;

        Info<< "    multi-component carrier - " << mcThermo.species().size()
            << " species" << endl;
    }
    else
    {
        Info<< "    single component carrier" << endl;
    }

    if (thermo.found("liquids"))
    {
        liquids_ = liquidMixture::New(thermo.subDict("liquids"));
        Info<< "    liquids - " << liquids_->components().size()
            << " components" << endl;
    }
    else
    {
        Info<< "    no liquid components" << endl;
    }

    if (thermo.found("solids"))
    {
        solids_  = solidMixture::New(thermo.subDict("solids"));
        Info<< "    solids - " << solids_->components().size()
            << " components" << endl;
    }
    else
    {
        Info<< "    no solid components" << endl;
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::SLGThermo::~SLGThermo()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const Foam::basicThermo& Foam::SLGThermo::thermo() const
{
    return thermo_;
}


const Foam::basicMultiComponentMixtureNew& Foam::SLGThermo::carrier() const
{
    if (carrier_ == NULL)
    {
        FatalErrorIn
        (
            "const Foam::basicMultiComponentMixtureNew& "
            "Foam::SLGThermo::carrier() const"
        )   << "carrier requested, but object is not allocated"
            << abort(FatalError);
    }

    return *carrier_;
}


const Foam::liquidMixture& Foam::SLGThermo::liquids() const
{
    if (!liquids_.valid())
    {
        FatalErrorIn
        (
            "const Foam::liquidMixture& Foam::SLGThermo::liquids() const"
        )   << "liquids requested, but object is not allocated"
            << abort(FatalError);
    }

    return liquids_();
}


const Foam::solidMixture& Foam::SLGThermo::solids() const
{
    if (!solids_.valid())
    {
        FatalErrorIn
        (
            "const Foam::solidMixture& Foam::SLGThermo::solids() const"
        )   << "solids requested, but object is not allocated"
            << abort(FatalError);
    }

    return solids_();
}


Foam::label Foam::SLGThermo::carrierId
(
    const word& cmptName,
    bool allowNotfound
) const
{
    forAll(carrier().species(), i)
    {
        if (cmptName == carrier_->species()[i])
        {
            return i;
        }
    }

    if (!allowNotfound)
    {
        FatalErrorIn
        (
            "Foam::label Foam::SLGThermo::carrierId(const word&, bool) const"
        )   << "Unknown carrier component " << cmptName
            << ". Valid carrier components are:" << nl
            << carrier_->species() << exit(FatalError);
    }

    return -1;
}


Foam::label Foam::SLGThermo::liquidId
(
    const word& cmptName,
    bool allowNotfound
) const
{
    forAll(liquids().components(), i)
    {
        if (cmptName == liquids_->components()[i])
        {
            return i;
        }
    }

    if (!allowNotfound)
    {
        FatalErrorIn
        (
            "Foam::label Foam::SLGThermo::liquidId(const word&, bool) const"
        )   << "Unknown liquid component " << cmptName << ". Valid liquids are:"
            << nl << liquids_->components() << exit(FatalError);
    }

    return -1;
}


Foam::label Foam::SLGThermo::solidId
(
    const word& cmptName,
    bool allowNotfound
) const
{
    forAll(solids().components(), i)
    {
        if (cmptName == solids_->components()[i])
        {
            return i;
        }
    }

    if (!allowNotfound)
    {
        FatalErrorIn
        (
            "Foam::label Foam::SLGThermo::solidId(const word&, bool) const"
        )   << "Unknown solid component " << cmptName << ". Valid solids are:"
            << nl << solids_->components() << exit(FatalError);
    }

    return -1;
}


bool Foam::SLGThermo::hasMultiComponentCarrier() const
{
    return (carrier_ != NULL);
}


bool Foam::SLGThermo::hasLiquids() const
{
    return liquids_.valid();
}


bool Foam::SLGThermo::hasSolids() const
{
    return solids_.valid();
}


// ************************************************************************* //

