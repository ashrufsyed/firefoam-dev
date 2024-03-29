/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2008-2010 OpenCFD Ltd.
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

#include "DevolatilisationModel.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::DevolatilisationModel<CloudType>::DevolatilisationModel
(
    CloudType& owner
)
:
    dict_(dictionary::null),
    owner_(owner),
    coeffDict_(dictionary::null)
{}


template<class CloudType>
Foam::DevolatilisationModel<CloudType>::DevolatilisationModel
(
    const dictionary& dict,
    CloudType& owner,
    const word& type
)
:
    dict_(dict),
    owner_(owner),
    coeffDict_(dict.subDict(type + "Coeffs"))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CloudType>
Foam::DevolatilisationModel<CloudType>::~DevolatilisationModel()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //
template<class CloudType>
const CloudType& Foam::DevolatilisationModel<CloudType>::owner() const
{
    return owner_;
}


template<class CloudType>
const Foam::dictionary& Foam::DevolatilisationModel<CloudType>::dict() const
{
    return dict_;
}


template<class CloudType>
const Foam::dictionary& Foam::DevolatilisationModel<CloudType>::coeffDict() const
{
    return coeffDict_;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "NewDevolatilisationModel.C"

// ************************************************************************* //

