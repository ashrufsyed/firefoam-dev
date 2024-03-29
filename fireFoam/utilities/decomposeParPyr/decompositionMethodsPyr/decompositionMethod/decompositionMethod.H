/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2010 OpenCFD Ltd.
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
    Foam::decompositionMethod

Description
    Abstract base class for decomposition

SourceFiles
    decompositionMethod.C

\*---------------------------------------------------------------------------*/

#ifndef decompositionMethod_H
#define decompositionMethod_H

#include "polyMesh.H"
#include "pointField.H"

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class decompositionMethod Declaration
\*---------------------------------------------------------------------------*/

class decompositionMethod
{

protected:

    // Protected data

        const dictionary& decompositionDict_;
        label nProcessors_;


        //- Helper: determine (non-parallel) cellCells from mesh agglomeration.
        static void calcCellCells
        (
            const polyMesh& mesh,
            const labelList& agglom,
            const label nCoarse,
            labelListList& cellCells
        );

private:

    // Private Member Functions

        //- Disallow default bitwise copy construct and assignment
        decompositionMethod(const decompositionMethod&);
        void operator=(const decompositionMethod&);


public:

    //- Runtime type information
    TypeName("decompositionMethod");


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            autoPtr,
            decompositionMethod,
            dictionary,
            (
                const dictionary& decompositionDict
            ),
            (decompositionDict)
        );

        declareRunTimeSelectionTable
        (
            autoPtr,
            decompositionMethod,
            dictionaryMesh,
            (
                const dictionary& decompositionDict,
                const polyMesh& mesh
            ),
            (decompositionDict, mesh)
        );


    // Selectors

        //- Return a reference to the selected decomposition method
        static autoPtr<decompositionMethod> New
        (
            const dictionary& decompositionDict
        );

        //- Return a reference to the selected decomposition method
        static autoPtr<decompositionMethod> New
        (
            const dictionary& decompositionDict,
            const polyMesh& mesh
        );


    // Constructors

        //- Construct given the decomposition dictionary
        decompositionMethod(const dictionary& decompositionDict)
        :
            decompositionDict_(decompositionDict),
            nProcessors_
            (
                readLabel(decompositionDict.lookup("numberOfSubdomains"))
            )
        {}


    // Destructor

        virtual ~decompositionMethod()
        {}


    // Member Functions
        label nDomains() const
        {
            return nProcessors_;
        }


        //- Is method parallel aware (i.e. does it synchronize domains across
        //  proc boundaries)
        virtual bool parallelAware() const = 0;

        //- Return for every coordinate the wanted processor number. Use the
        //  mesh connectivity (if needed)
        virtual labelList decompose
        (
            const pointField& points,
            const scalarField& pointWeights
        ) = 0;

        //- Like decompose but with uniform weights on the points
        virtual labelList decompose(const pointField&);


        //- Return for every coordinate the wanted processor number. Gets
        //  passed agglomeration map (from fine to coarse cells) and coarse cell
        //  location. Can be overridden by decomposers that provide this
        //  functionality natively. Coarse cells are local to the processor
        //  (if in parallel). If you want to have coarse cells spanning
        //  processors use the globalCellCells instead.
        virtual labelList decompose
        (
            const labelList& cellToRegion,
            const pointField& regionPoints,
            const scalarField& regionWeights
        );

        //- Like decompose but with uniform weights on the regions
        virtual labelList decompose
        (
            const labelList& cellToRegion,
            const pointField& regionPoints
        );


        //- Return for every coordinate the wanted processor number. Explicitly
        //  provided connectivity - does not use mesh_.
        //  The connectivity is equal to mesh.cellCells() except for
        //  - in parallel the cell numbers are global cell numbers (starting
        //    from 0 at processor0 and then incrementing all through the
        //    processors)
        //  - the connections are across coupled patches
        virtual labelList decompose
        (
            const labelListList& globalCellCells,
            const pointField& cc,
            const scalarField& cWeights
        ) = 0;

        //- Like decompose but with uniform weights on the cells
        virtual labelList decompose
        (
            const labelListList& globalCellCells,
            const pointField& cc
        );

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
