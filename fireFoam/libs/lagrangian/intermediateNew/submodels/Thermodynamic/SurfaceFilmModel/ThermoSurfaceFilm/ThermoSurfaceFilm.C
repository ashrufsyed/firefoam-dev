/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2009-2010 OpenCFD Ltd.
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
#define DEBUG(x) std::cout << "["<< __FILE__ << ":" << __LINE__ << "] "<< #x " = " << x << std::endl;
#define TRACE(s) std::cout << "["<< __FILE__ << ":" << __LINE__ << "] "<< #s << std::endl; s;

#include "ThermoSurfaceFilm.H"
#include "addToRunTimeSelectionTable.H"
#include "mathematicalConstants.H"
#include "Pstream.H"

using namespace Foam::mathematicalConstant;

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

template<class CloudType>
Foam::wordList Foam::ThermoSurfaceFilm<CloudType>::interactionTypeNames_
(
    IStringStream
    (
        "(absorb bounce splashBai)"
    )()
);


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

template<class CloudType>
typename Foam::ThermoSurfaceFilm<CloudType>::interactionType
Foam::ThermoSurfaceFilm<CloudType>::interactionTypeEnum(const word& it) const
{
    forAll(interactionTypeNames_, i)
    {
        if (interactionTypeNames_[i] == it)
        {
            return interactionType(i);
        }
    }

    FatalErrorIn
    (
        "ThermoSurfaceFilm<CloudType>::interactionType "
        "ThermoSurfaceFilm<CloudType>::interactionTypeEnum"
        "("
            "const word& it"
        ") const"
    )   << "Unknown interaction type " << it
        << ". Valid interaction types include: " << interactionTypeNames_
        << abort(FatalError);

    return interactionType(0);
}


template<class CloudType>
Foam::word Foam::ThermoSurfaceFilm<CloudType>::interactionTypeStr
(
    const interactionType& it
) const
{
    if (it >= interactionTypeNames_.size())
    {
        FatalErrorIn
        (
            "ThermoSurfaceFilm<CloudType>::interactionType "
            "ThermoSurfaceFilm<CloudType>::interactionTypeStr"
            "("
                "const interactionType& it"
            ") const"
        )   << "Unknown interaction type enumeration" << abort(FatalError);
    }

    return interactionTypeNames_[it];
}


template<class CloudType>
Foam::vector Foam::ThermoSurfaceFilm<CloudType>::tangentVector
(
    const vector& v
) const
{
    vector tangent = vector::zero;
    scalar magTangent = 0.0;

    while (magTangent < SMALL)
    {
        vector vTest = rndGen_.vector01();
        tangent = vTest - (vTest & v)*v;
        magTangent = mag(tangent);
    }

    return tangent/magTangent;
}


template<class CloudType>
Foam::vector Foam::ThermoSurfaceFilm<CloudType>::splashDirection
(
    const vector& tanVec1,
    const vector& tanVec2,
    const vector& nf
) const
{
    // azimuthal angle [rad]
    const scalar phiSi = twoPi*rndGen_.scalar01();

    // ejection angle [rad]
    const scalar thetaSi = pi/180.0*(rndGen_.scalar01()*(50 - 5) + 5);

    // direction vector of new parcel
    const scalar alpha = sin(thetaSi);
    const scalar dcorr = cos(thetaSi);
    const vector normal = alpha*(tanVec1*cos(phiSi) + tanVec2*sin(phiSi));
    vector dirVec = dcorr*nf;
    dirVec += normal;

    return dirVec/mag(dirVec);
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::absorbInteraction
(
    regionModels::surfaceFilmModels::surfaceFilmModel& filmModel,
    const parcelType& p,
    const polyPatch& pp,
    const label faceI,
    const scalar mass
)
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " absorbInteraction" << endl;
        Info << "energy " << mass*p.hs() << endl;                     // energy
        Info << "enthalpy (J/kg) " << p.hs() << endl;                     // energy
    }

    // Patch face normal
    const vector& nf = pp.faceNormals()[faceI];

    // Patch velocity
    const vector& Up = this->owner().U().boundaryField()[pp.index()][faceI];

    // Relative parcel velocity
    const vector Urel = p.U() - Up;

    // Parcel normal velocity
    const vector Un = nf*(Urel & nf);

    // Parcel tangential velocity
    const vector Ut = Urel - Un;

    filmModel.addSources
    (
        pp.index(),
        faceI,
        mass,                           // mass
        mass*Ut,                        // tangential momentum
        mass*mag(Un),                   // impingement pressure
        mass*p.hs()                     // energy
    );

    this->nParcelsTransferred()++;
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::bounceInteraction
(
    parcelType& p,
    const polyPatch& pp,
    const label faceI
) const
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " bounceInteraction" << endl;
    }

    // Patch face normal
    const vector& nf = pp.faceNormals()[faceI];

    // Patch velocity
    const vector& Up = this->owner().U().boundaryField()[pp.index()][faceI];

    // Relative parcel velocity
    const vector Urel = p.U() - Up;

    // Flip parcel normal velocity component
    p.U() -= 2.0*nf*(Urel & nf);
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::drySplashInteraction
(
    regionModels::surfaceFilmModels::surfaceFilmModel& filmModel,
    const parcelType& p,
    const polyPatch& pp,
    const label faceI
)
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " drySplashInteraction" << endl;
    }

    const liquid& liq = thermo_.liquids().properties()[0];

    // Patch face velocity and normal
    const vector& Up = this->owner().U().boundaryField()[pp.index()][faceI];
    const vector& nf = pp.faceNormals()[faceI];

    // local pressure
    const scalar pc = thermo_.thermo().p()[p.cell()];

    // Retrieve parcel properties
    const scalar m = p.mass()*p.nParticle();
    const scalar rho = p.rho();
    const scalar d = p.d();
    const scalar sigma = liq.sigma(pc, p.T());
    const scalar mu = liq.mu(pc, p.T());
    const vector Urel = p.U() - Up;
    const vector Un = nf*(Urel & nf);

    // Laplace number
    const scalar La = rho*sigma*d/sqr(mu);

    // Weber number
    const scalar We = rho*magSqr(Un)*d/sigma;

    // Critical Weber number
    const scalar Wec = Adry_*pow(La, -0.183);

    if (We < Wec) // adhesion - assume absorb
    {
        absorbInteraction(filmModel, p, pp, faceI, m);
    }
    else // splash
    {
        // ratio of incident mass to splashing mass
        const scalar mRatio = 0.2 + 0.6*rndGen_.scalar01();
        splashInteraction(filmModel, p, pp, faceI, mRatio, We, Wec, sigma);
    }
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::wetSplashInteraction
(
    regionModels::surfaceFilmModels::surfaceFilmModel& filmModel,
    parcelType& p,
    const polyPatch& pp,
    const label faceI,
    bool& keepParticle
)
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " wetSplashInteraction" << endl;
    }

    const liquid& liq = thermo_.liquids().properties()[0];

    // Patch face velocity and normal
    const vector& Up = this->owner().U().boundaryField()[pp.index()][faceI];
    const vector& nf = pp.faceNormals()[faceI];

    // local pressure
    const scalar pc = thermo_.thermo().p()[p.cell()];

    // Retrieve parcel properties
    const scalar m = p.mass()*p.nParticle();
    const scalar rho = p.rho();
    const scalar d = p.d();
    vector& U = p.U();
    const scalar sigma = liq.sigma(pc, p.T());
    const scalar mu = liq.mu(pc, p.T());
    const vector Urel = p.U() - Up;
    const vector Un = nf*(Urel & nf);
    const vector Ut = Urel - Un;

    // Laplace number
    const scalar La = rho*sigma*d/sqr(mu);

    // Weber number
    const scalar We = rho*magSqr(Un)*d/sigma;

    // Critical Weber number
    const scalar Wec = Awet_*pow(La, -0.183);

    if (We < 1) // adhesion - assume absorb
    {
        absorbInteraction(filmModel, p, pp, faceI, m);
        keepParticle = false;
    }
    else if ((We >= 1) && (We < 20)) // bounce
    {
        // incident angle of impingement
        const scalar theta = pi/2 - acos(U/mag(U) & nf);

        // restitution coefficient
        const scalar epsilon = 0.993 - theta*(1.76 - theta*(1.56 - theta*0.49));

        // update parcel velocity
        U = -epsilon*(Un) + 5/7*(Ut);
        keepParticle = true;
    }
    else if ((We >= 20) && (We < Wec)) // spread - assume absorb
    {
        absorbInteraction(filmModel, p, pp, faceI, m);
        keepParticle = false;
    }
    else    // splash
    {
        // ratio of incident mass to splashing mass
        // splash mass can be > incident mass due to film entrainment
        scalar mRatio = 0.2 + 0.9*rndGen_.scalar01();
        splashInteraction(filmModel, p, pp, faceI, mRatio, We, Wec, sigma);
        keepParticle = false;
    }
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::splashInteraction
(
    regionModels::surfaceFilmModels::surfaceFilmModel& filmModel,
    const parcelType& p,
    const polyPatch& pp,
    const label faceI,
    const scalar mRatio,
    const scalar We,
    const scalar Wec,
    const scalar sigma
)
{
    // Patch face velocity and normal
    const vector& Up = this->owner().U().boundaryField()[pp.index()][faceI];
    const vector& nf = pp.faceNormals()[faceI];

    // Determine direction vectors tangential to patch normal
    const vector tanVec1 = tangentVector(nf);
    const vector tanVec2 = nf^tanVec1;

    // Retrieve parcel properties
    const scalar np = p.nParticle();
    const scalar m = p.mass()*np;
    const scalar rho = p.rho();
    const scalar d = p.d();
    const vector Urel = p.U() - Up;
    const vector Un = nf*(Urel & nf);
    const vector Ut = Urel - Un;
    const vector& posC = this->mesh().C()[p.cell()];
    const vector& posCf = this->mesh().Cf().boundaryField()[pp.index()][faceI];

    // total mass of (all) splashed parcels
    const scalar mSplash = m*mRatio;

    // number of splashed particles per incoming particle
    const scalar Ns = 5.0*(We/Wec - 1.0);

    // average diameter of splashed particles
    const scalar dBarSplash = 1/cbrt(6.0)*cbrt(mRatio/Ns)*d + ROOTVSMALL;

    // cumulative diameter splash distribution
    const scalar dMin = 0.1*d;
    const scalar dMax = cbrt(6.0*mSplash/np/rho/pi);
    //kvm, const scalar dMax = d;
    const scalar K = exp(-dMin/dBarSplash) - exp(-dMax/dBarSplash);

    // surface energy of secondary parcels [J]
    scalar ESigmaSec = 0;

    // sample splash distribution to detrmine secondary parcel diameters
    scalarList dNew(parcelsPerSplash_);
    forAll(dNew, i)
    {
        const scalar y = rndGen_.scalar01();
        dNew[i] = max(dMin,-dBarSplash*log(exp(-dMin/dBarSplash) - y*K+SMALL));//have andy update to this
        ESigmaSec += sigma*p.areaS(dNew[i]);//kvm, we don't need to multiply by ptr->np here?
        /*Info << "diameter " << dNew[i]<<endl;*/
    }

    // incident kinetic energy [J]
    const scalar EKIn = 0.5*m*magSqr(Urel);

    // incident surface energy [J]
    const scalar ESigmaIn = sigma*p.areaS(d);//kvm, we don't need to multiply by np here?

    // dissipative energy
    const scalar Ed = max(0.8*EKIn, Wec/12*pi*sigma*sqr(d));

    // total energy [J]
    const scalar EKs = EKIn + ESigmaIn - ESigmaSec - Ed;

    // switch to absorb if insufficient energy for splash
    if (EKs <= 0)
    {
        absorbInteraction(filmModel, p, pp, faceI, m);
        return;
    }

    // helper variables to calculate magUns0
    const scalar logD = log(d);
    const scalar coeff2 = log(dNew[0]) - logD + ROOTVSMALL;
    scalar coeff1 = 0.0;
    forAll(dNew, i)
    {
        coeff1 += sqr(log(dNew[i]) - logD);
    }

    // magnitude of the normal velocity of the first splashed parcel
    const scalar magUns0 =
        sqrt(2.0*parcelsPerSplash_*EKs/mSplash/(1 + coeff1/sqr(coeff2)));

    // Set splashed parcel properties
    forAll(dNew, i)
    {
        const vector dirVec = splashDirection(tanVec1, tanVec2, -nf);

        // Create a new parcel by copying source parcel
        parcelType* pPtr = new parcelType(p);

//        pPtr->origId() = this->owner().getNewParticleID();

//        pPtr->orgProc() = Pstream::myProcNo();

        if (splashParcelType_ < 0)
        {
            pPtr->typeId()++;
        }
        else
        {
            pPtr->typeId() = splashParcelType_;
        }

        // perturb new parcels towards the owner cell centre
        pPtr->position() = p.position() + 0.5*rndGen_.scalar01()*(posC - posCf);

        pPtr->nParticle() = mRatio*np*pow3(d)/pow3(dNew[i])/parcelsPerSplash_;

        pPtr->d() = dNew[i];

        pPtr->U() = dirVec*(mag(Cf_*Ut) + magUns0*(log(dNew[i]) - logD)/coeff2);

        // Apply correction to velocity for 2-D cases
        meshTools::constrainDirection
        (
            this->mesh(),
            this->mesh().solutionD(),
            pPtr->U()
        );

        // Add the new parcel
        this->owner().addParticle(pPtr);

        nParcelsSplashed_++;
    }

    // transfer remaining part of parcel to film 0 - splashMass can be -ve
    // if entraining from the film
    const scalar mDash = m - mSplash;
    absorbInteraction(filmModel, p, pp, faceI, mDash);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::ThermoSurfaceFilm<CloudType>::ThermoSurfaceFilm
(
    const dictionary& dict,
    CloudType& owner,
    const dimensionedVector& g
)
:
    SurfaceFilmModel<CloudType>(dict, owner, g, typeName),
    rndGen_(owner.rndGen()),
    thermo_(this->mesh().objectRegistry::lookupObject<SLGThermo>("SLGThermo")),
    TFilmPatch_(0),
    cpFilmPatch_(0),
    interactionType_
    (
        interactionTypeEnum(this->coeffDict().lookup("interactionType"))
    ),
    deltaWetThreshold_(0.0),
    splashParcelType_(0),
    parcelsPerSplash_(0),
    Adry_(0.0),
    Awet_(0.0),
    Cf_(0.0),
    nParcelsSplashed_(0)
{
    Info<< "    Applying " << interactionTypeStr(interactionType_)
        << " interaction model" << endl;

    if (interactionType_ == itSplashBai)
    {
        this->coeffDict().lookup("deltaWetThreshold") >> deltaWetThreshold_;
        splashParcelType_ =
            this->coeffDict().lookupOrDefault("splashParcelType", -1);
        parcelsPerSplash_ =
            this->coeffDict().lookupOrDefault("parcelsPerSplash", 2);
        this->coeffDict().lookup("Adry") >> Adry_;
        this->coeffDict().lookup("Awet") >> Awet_;
        this->coeffDict().lookup("Cf") >> Cf_;
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CloudType>
Foam::ThermoSurfaceFilm<CloudType>::~ThermoSurfaceFilm()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
bool Foam::ThermoSurfaceFilm<CloudType>::active() const
{
    return true;
}


template<class CloudType>
bool Foam::ThermoSurfaceFilm<CloudType>::transferParcel
(
    parcelType& p,
    const polyPatch& pp,
    bool& keepParticle
)
{
    // Retrieve the film model from the owner database
    regionModels::surfaceFilmModels::surfaceFilmModel& filmModel =
        const_cast<regionModels::surfaceFilmModels::surfaceFilmModel&>
        (
            this->owner().db().objectRegistry::
                lookupObject<regionModels::surfaceFilmModels::surfaceFilmModel>
                (
                    "surfaceFilmProperties"
                )
        );

    const label patchI = pp.index();

    if (filmModel.isRegionPatch(patchI))
    {
        const label faceI = pp.whichFace(p.face());

        switch (interactionType_)
        {
            case itBounce:
            {
                bounceInteraction(p, pp, faceI);
                keepParticle = true;

                break;
            }
            case itAbsorb:
            {
                const scalar m = p.nParticle()*p.mass();
                absorbInteraction(filmModel, p, pp, faceI, m);
                keepParticle = false;

                break;
            }
            case itSplashBai:
            {
                bool dry =
                    this->deltaFilmPatch_[patchI][faceI] < deltaWetThreshold_;

                if (dry)
                {
                    drySplashInteraction(filmModel, p, pp, faceI);
                    keepParticle = false;
                }
                else
                {
                    wetSplashInteraction(filmModel, p, pp, faceI, keepParticle);
                }

                break;
            }
            default:
            {
                FatalErrorIn
                (
                    "bool ThermoSurfaceFilm<CloudType>::transferParcel"
                    "("
                        "const parcelType&, "
                        "const label"
                    ")"
                )   << "Unknown interaction type enumeration"
                    << abort(FatalError);
            }
        }

        // transfer parcel/parcel interactions complete
        return true;
    }

    // parcel not interacting with film
    return false;
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::cacheFilmFields
(
    const label filmPatchI,
    const label primaryPatchI,
    const mapDistribute& distMap,
    const regionModels::surfaceFilmModels::surfaceFilmModel& filmModel
)
{
    SurfaceFilmModel<CloudType>::cacheFilmFields
    (
        filmPatchI,
        primaryPatchI,
        distMap,
        filmModel
    );

    TFilmPatch_ = filmModel.T().boundaryField()[filmPatchI];
    distMap.distribute(TFilmPatch_);

    cpFilmPatch_ = filmModel.Cp().boundaryField()[filmPatchI];
    distMap.distribute(cpFilmPatch_);
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::setParcelProperties
(
    parcelType& p,
    const label filmFaceI
) const
{
    SurfaceFilmModel<CloudType>::setParcelProperties(p, filmFaceI);

    // Set parcel properties
    p.T() = TFilmPatch_[filmFaceI];
    p.cp() = cpFilmPatch_[filmFaceI];
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::info(Ostream& os) const
{
    os  << "    Parcels absorbed into film      = "
        << returnReduce(this->nParcelsTransferred(), sumOp<label>()) << nl
        << "    New film detached parcels       = "
        << returnReduce(this->nParcelsInjected(), sumOp<label>()) << nl
        << "    New film splash parcels         = "
        << returnReduce(nParcelsSplashed_, sumOp<label>()) << nl;
}


// ************************************************************************* //
