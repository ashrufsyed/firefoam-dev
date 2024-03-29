/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2010-2010 OpenCFD Ltd.
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

Class
    Foam::structuredDecomp

Description
    Decomposition given using consecutive application of decomposers.

SourceFiles
    structuredDecomp.C

\*---------------------------------------------------------------------------*/

#ifndef structuredDecomp_H
#define structuredDecomp_H

#include "fvMeshSubset.H"
#include "decompositionMethod.H"

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class structuredDecomp Declaration
\*---------------------------------------------------------------------------*/

class structuredDecomp
:
    public decompositionMethod
{
    // Private data
        const polyMesh& mesh_;

        dictionary methodDict_;

        fvMeshSubset subsetter_;

        autoPtr<decompositionMethod> method_;

        wordList patches_;


    // Private Member Functions


        //- Disallow default bitwise copy construct and assignment
        void operator=(const structuredDecomp&);
        structuredDecomp(const structuredDecomp&);


public:

    //- Runtime type information
    TypeName("structured");


    // Constructors

         //- Construct given the decomposition dictionary and mesh
        structuredDecomp
        (
            const dictionary& decompositionDict,
            const polyMesh& mesh
        );


    //- Destructor
    virtual ~structuredDecomp()
    {}


    // Member Functions

        //- Is method parallel aware (i.e. does it synchronize domains across
        //  proc boundaries)
        virtual bool parallelAware() const;


        virtual labelList decompose
        (
            const pointField& points,
            const scalarField& pointWeights
        );


        virtual labelList decompose
        (
            const labelListList& globalCellCells,
            const pointField& cc,
            const scalarField& cWeights
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
