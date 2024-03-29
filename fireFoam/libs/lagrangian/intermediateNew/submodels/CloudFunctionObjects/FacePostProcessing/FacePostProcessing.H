/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2011 OpenCFD Ltd.
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
    Foam::FacePostProcessing

Description
    Records particle face quantities on used-specified face zone

    Currently supports:
        accummulated mass
        average mass flux

SourceFiles
    FacePostProcessing.C

\*---------------------------------------------------------------------------*/

#ifndef FacePostProcessing_H
#define FacePostProcessing_H

#include "CloudFunctionObject.H"
#include "faceZone.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Class FacePostProcessing Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class FacePostProcessing
:
    public CloudFunctionObject<CloudType>
{
    // Private Data

        // Typedefs

            //- Convenience typedef for parcel type
            typedef typename CloudType::parcelType parcelType;


        //- Face zone IDs
        labelList faceZoneIDs_;

        //- Surface output format
        const word surfaceFormat_;

        //- Flag to indicate whether data should be reset/cleared on writing
        bool resetOnWrite_;

        //- Flag to indicate whether data should be written to file
        bool log_;

        //- Total time
        scalar totalTime_;

        //- Mass storage
        List<scalarField> mass_;

        //- Mass total storage
        List<scalarField> massTotal_;

        //- Mass flux storage
        List<scalarField> massFlux_;

        //- Output file pointer per zone
        PtrList<OFstream> outputFilePtr_;

        //- Output directory
        fileName outputDir_;


    // Private Member Functions

        //- Helper function to create log files
        void makeLogFile
        (
            const word& zoneName,
            const label zoneI,
            const label nFaces,
            const scalar totArea
        );

        //- Return index into storage lists if valid zone and face
        void applyToFace
        (
            const label faceIn,
            label& zoneI, label&
            faceI
        ) const;


protected:

    // Protected Member Functions

        //- Write post-processing info
        void write();


public:

    //- Runtime type information
    TypeName("facePostProcessing");


    // Constructors

        //- Construct from dictionary
        FacePostProcessing(const dictionary& dict, CloudType& owner);


    //- Destructor
    virtual ~FacePostProcessing();


    // Member Functions

        // Access

            //- Return const access to the reset on write flag
            inline const Switch& resetOnWrite() const;


        // Evaluation

            //- Post-patch hook
            virtual void postPatch(const parcelType& p, const label patchI);

            //- Post-face hook
            virtual void postFace(const parcelType& p);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "FacePostProcessingI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "FacePostProcessing.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
