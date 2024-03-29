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

\*---------------------------------------------------------------------------*/

#include "structuredDecomp.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"
#include "FaceCellWave.H"
#include "topoDistanceData.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(structuredDecomp, 0);

    addToRunTimeSelectionTable
    (
        decompositionMethod,
        structuredDecomp,
        dictionaryMesh
    );
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::structuredDecomp::structuredDecomp
(
    const dictionary& decompositionDict,
    const polyMesh& mesh
)
:
    decompositionMethod(decompositionDict),
    mesh_(mesh),
    methodDict_(decompositionDict.subDict(typeName + "Coeffs")),
    subsetter_(dynamic_cast<const fvMesh&>(mesh_))
{

    //dictionary myDict = decompositionDict_.subDict(typeName + "Coeffs");
    methodDict_.set("numberOfSubdomains", nDomains());
    patches_ = wordList(methodDict_.lookup("patches"));

    labelList patchIDs(patches_.size());
    const polyBoundaryMesh& pbm = mesh_.boundaryMesh();

    label nFaces = 0;
    forAll(patches_, i)
    {
        patchIDs[i] = pbm.findPatchID(patches_[i]);

        if (patchIDs[i] == -1)
        {
            FatalErrorIn("structuredDecomp::decompose(..)")
                << "Cannot find patch " << patches_[i] << endl
                << "Valid patches are " << pbm.names()
                << exit(FatalError);
        }
        nFaces += pbm[patchIDs[i]].size();
    }

    // Extract a submesh.
    labelHashSet patchCells(2*nFaces);
    forAll(patchIDs, i)
    {
        const unallocLabelList& fc = pbm[patchIDs[i]].faceCells();
        forAll(fc, i)
        {
            patchCells.insert(fc[i]);
        }
    }

    subsetter_.setLargeCellSubset(patchCells);

    method_ = decompositionMethod::New(methodDict_, subsetter_.subMesh());

}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::structuredDecomp::parallelAware() const
{
    return method_().parallelAware();
}


Foam::labelList Foam::structuredDecomp::decompose
(
    const pointField& cc,
    const scalarField& cWeights
)
{
    labelList patchIDs(patches_.size());
    const polyBoundaryMesh& pbm = mesh_.boundaryMesh();

    label nFaces = 0;
    forAll(patches_, i)
    {
        patchIDs[i] = pbm.findPatchID(patches_[i]);

        if (patchIDs[i] == -1)
        {
            FatalErrorIn("structuredDecomp::decompose(..)")
                << "Cannot find patch " << patches_[i] << endl
                << "Valid patches are " << pbm.names()
                << exit(FatalError);
        }
        nFaces += pbm[patchIDs[i]].size();
    }
// 
//     // Extract a submesh.
//     labelHashSet patchCells(2*nFaces);
//     forAll(patchIDs, i)
//     {
//         const unallocLabelList& fc = pbm[patchIDs[i]].faceCells();
//         forAll(fc, i)
//         {
//             patchCells.insert(fc[i]);
//         }
//     }

    // Subset the layer of cells next to the patch
    //fvMeshSubset subsetter(dynamic_cast<const fvMesh&>(mesh_));
    //subsetter_.setLargeCellSubset(patchCells);
    //const fvMesh& subMesh = subsetter_.subMesh();
    pointField subCc(cc, subsetter_.cellMap());
    scalarField subWeights(cWeights, subsetter_.cellMap());

    // Decompose the layer of cells
    labelList subDecomp(method_().decompose(subCc));


    // Transfer to final decomposition
    labelList finalDecomp(cc.size(), -1);
    forAll(subDecomp, i)
    {
        finalDecomp[subsetter_.cellMap()[i]] = subDecomp[i];
    }

    // Field on cells and faces.
    List<topoDistanceData> cellData(mesh_.nCells());
    List<topoDistanceData> faceData(mesh_.nFaces());

    // Start of changes
    labelList patchFaces(nFaces);
    List<topoDistanceData> patchData(nFaces);
    nFaces = 0;
    forAll(patchIDs, i)
    {
        const polyPatch& pp = pbm[patchIDs[i]];
        const unallocLabelList& fc = pp.faceCells();
        forAll(fc, i)
        {
            patchFaces[nFaces] = pp.start()+i;
            patchData[nFaces] = topoDistanceData(finalDecomp[fc[i]], 0);
            nFaces++;
        }
    }

    // Propagate information inwards
    FaceCellWave<topoDistanceData> deltaCalc
    (
        mesh_,
        patchFaces,
        patchData,
        faceData,
        cellData,
        mesh_.globalData().nTotalCells()
    );

    // And extract
    bool haveWarned = false;
    forAll(finalDecomp, cellI)
    {
        if (!cellData[cellI].valid())
        {
            if (!haveWarned)
            {
                WarningIn("structuredDecomp::decompose(..)")
                    << "Did not visit some cells, e.g. cell " << cellI
                    << " at " << mesh_.cellCentres()[cellI] << endl
                    << "Assigning  these cells to domain 0." << endl;
                haveWarned = true;
            }
            finalDecomp[cellI] = 0;
        }
        else
        {
            finalDecomp[cellI] = cellData[cellI].data();
        }
    }

    return finalDecomp;
}


Foam::labelList Foam::structuredDecomp::decompose
(
    const labelListList& globalPointPoints,
    const pointField& points,
    const scalarField& pointWeights
)
{
    notImplemented
    (
        "structuredDecomp::decompose\n"
        "(\n"
        "    const labelListList&,\n"
        "    const pointField&,\n"
        "    const scalarField&\n"
        ")\n"
    );

    return labelList::null();
}


// ************************************************************************* //
