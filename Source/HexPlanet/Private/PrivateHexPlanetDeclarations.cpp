
#include "HexPlanet.h"
#include "PrivateHexPlanetDeclarations.h"



FString createKeyForVector(const FVector& vectorKey)
{
	FVector unitVec;
	float mag;
	vectorKey.ToDirectionAndLength(unitVec, mag);
	FVector2D spherical = unitVec.UnitCartesianToSpherical();
	return FString::FromInt(spherical.X * 10000) + ", " + FString::FromInt(spherical.Y * 10000);
}
