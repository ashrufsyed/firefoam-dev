/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2010 OpenCFD Ltd.
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

inline Foam::PtrList<Foam::volScalarField>&
Foam::basicMultiComponentMixtureNew::Y()
{
    return Y_;
}


inline const Foam::PtrList<Foam::volScalarField>&
Foam::basicMultiComponentMixtureNew::Y() const
{
    return Y_;
}


inline Foam::volScalarField& Foam::basicMultiComponentMixtureNew::Y
(
    const label i
)
{
    return Y_[i];
}


inline const Foam::volScalarField& Foam::basicMultiComponentMixtureNew::Y
(
    const label i
) const
{
    return Y_[i];
}


inline Foam::volScalarField& Foam::basicMultiComponentMixtureNew::Y
(
    const word& specieName
)
{
    return Y_[species_[specieName]];
}


inline const Foam::volScalarField& Foam::basicMultiComponentMixtureNew::Y
(
    const word& specieName
) const
{
    return Y_[species_[specieName]];
}


inline bool Foam::basicMultiComponentMixtureNew::contains
(
    const word& specieName
) const
{
    return species_.contains(specieName);
}


inline Foam::scalar Foam::basicMultiComponentMixtureNew::fres
(
    const scalar ft,
    const scalar stoicRatio
) const
{
    return max(ft - (1.0 - ft)/stoicRatio, 0.0);
}


inline Foam::tmp<Foam::volScalarField> Foam::basicMultiComponentMixtureNew::fres
(
    const volScalarField& ft,
    const dimensionedScalar& stoicRatio
) const
{
    return max(ft - (scalar(1) - ft)/stoicRatio.value(), scalar(0));
}


// ************************************************************************* //
