// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "SphereGrid.h"
#include <limits>


// Sets default values for this component's properties
USphereGrid::USphereGrid()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;
	numSubvisions = 2;
	gridFrequency = int32(FMath::Pow(3, numSubvisions));
	numNodes = 12 + 30 * (gridFrequency - 1) + (gridFrequency > 2 ? 10 * gridFrequency*(gridFrequency + 1) : 0);
	// ...
}


// Called when the game starts
void USphereGrid::BeginPlay()
{
	Super::BeginPlay();

	//creating base icosahedron
	TArray<FVector> nodeLocations = createBaseIcosahedron();

	// setup grid
	gridFrequency = int32(FMath::Pow(3, numSubvisions));
	numNodes = 12 + 30 * (gridFrequency - 1) + (gridFrequency > 2 ? 10 * gridFrequency*(gridFrequency + 1) : 0);
	gridLocationsM.SetNumZeroed(numNodes);
	gridPositions.SetNum(5*gridFrequency);
	int32 tileNum = 0;
	for (int32 uLoc = 0; uLoc < gridPositions.Num(); ++uLoc)
	{
		int32 vSize = 2 * gridFrequency + 1 + (gridFrequency % 3 == 0 ? gridFrequency : 0);
		gridPositions[uLoc].SetNumZeroed(vSize);
		for (int32 vLoc = 0; vLoc < vSize; vLoc++)
		{
			//establish the mapping of duplicate points
			FRectGridIndex newIndex;
			newIndex.uPos = uLoc;
			newIndex.vPos = vLoc;
			if (vLoc == 0 && uLoc != 0)
			{
				if (uLoc % gridFrequency == 0)
				{
					gridPositions[uLoc][vLoc] = gridPositions[0][0];
				}
				else if (uLoc % gridFrequency != 0)
				{
					gridPositions[uLoc][vLoc] = gridPositions[uLoc / gridFrequency][uLoc%gridFrequency];
				}
			}
			else if (vLoc == vSize-1 && uLoc != 0)
			{
				if (uLoc % gridFrequency == 0)
				{
					gridPositions[uLoc][vLoc] = gridPositions[0][vLoc];
				}
				else if (uLoc % gridFrequency != 0)
				{
					gridPositions[uLoc][vLoc] = gridPositions[uLoc - vLoc % gridFrequency][(vLoc/gridFrequency) * gridFrequency];
				}
			}
			else
			{
				gridPositions[uLoc][vLoc] = tileNum;
				gridLocationsM[gridPositions[uLoc][vLoc]].tileIndex = tileNum;
				++tileNum;
			}
			gridLocationsM[gridPositions[uLoc][vLoc]].gridPositions.Add(newIndex);

			//add this point as a reference point if it doesn't already exist
			if (uLoc%3==0 && vLoc%3==0)
			{
				if (!gridReferencePointsM.Contains(gridPositions[uLoc][vLoc]))
				{
					//order needs to be 1,8,4,2,6,9,3,11,7,5,10,0
					switch (gridReferencePointsM.Num())
					{
					case 0:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[1]);
						break;
					case 1:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[8]);
						break;
					case 2:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[4]);
						break;
					case 3:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[2]);
						break;
					case 4:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[6]);
						break;
					case 5:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[9]);
						break;
					case 6:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[3]);
						break;
					case 7:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[11]);
						break;
					case 8:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[7]);
						break;
					case 9:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[5]);
						break;
					case 10:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[10]);
						break;
					case 11:
						gridReferencePointsM.Add(gridPositions[uLoc][vLoc], nodeLocations[0]);
						break;
					}
				}
			}
		}
	}
}


TArray<FVector> USphereGrid::createBaseIcosahedron() const
{
	float x = 1;
	float z = (1 + FMath::Sqrt(5)) / 2.0;
	float baseHedronRadius = FMath::Sqrt(x*x + z*z);
	float pointMagnitudeRatio = 1.0 / baseHedronRadius;
	x *= pointMagnitudeRatio;
	z *= pointMagnitudeRatio;

	TArray<FVector> nodeLocations;
	nodeLocations.SetNumZeroed(12);

	nodeLocations[0][0] = 0;
	nodeLocations[0][1] = x;
	nodeLocations[0][2] = z;

	nodeLocations[1][0] = 0;
	nodeLocations[1][1] = x;
	nodeLocations[1][2] = -z;

	nodeLocations[2][0] = 0;
	nodeLocations[2][1] = -x;
	nodeLocations[2][2] = z;

	nodeLocations[3][0] = 0;
	nodeLocations[3][1] = -x;
	nodeLocations[3][2] = -z;

	nodeLocations[4][0] = z;
	nodeLocations[4][1] = 0;
	nodeLocations[4][2] = x;

	nodeLocations[5][0] = -z;
	nodeLocations[5][1] = 0;
	nodeLocations[5][2] = x;

	nodeLocations[6][0] = z;
	nodeLocations[6][1] = 0;
	nodeLocations[6][2] = -x;

	nodeLocations[7][0] = -z;
	nodeLocations[7][1] = 0;
	nodeLocations[7][2] = -x;

	nodeLocations[8][0] = x;
	nodeLocations[8][1] = z;
	nodeLocations[8][2] = 0;

	nodeLocations[9][0] = x;
	nodeLocations[9][1] = -z;
	nodeLocations[9][2] = 0;

	nodeLocations[10][0] = -x;
	nodeLocations[10][1] = z;
	nodeLocations[10][2] = 0;

	nodeLocations[11][0] = -x;
	nodeLocations[11][1] = -z;
	nodeLocations[11][2] = 0;
	return nodeLocations;
}

FRectGridLocation USphereGrid::getOffsetTile(const int32& startU, const int32& startV, const int32& offsetU, const int32& offsetV) const
{
	//simple case first
	int32 newU = startU + offsetU;
	int32 newV = startV + offsetV;
}

// Called every frame
void USphereGrid::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

FRectGridLocation USphereGrid::mapPosToTile(const FVector& positionOnSphere) const
{

}

FVector USphereGrid::getNodeLocationOnSphere(const int32& uLoc, const int32& vLoc) const
{
	//find the reference vectors
	int32 uRef1 = (uLoc / gridFrequency)*gridFrequency;
	int32 uRef2 = uRef1 + gridFrequency;
	//adjust for warp around
	if (uRef2 >= 5*gridFrequency)
	{
		uRef2 = gridFrequency;
	}
	//in the case of our key rows we actually want to reference the
	//preceding urefLocation instead of the next one to make it match
	//with how we're doing the vRef locations
	if (uRef1%gridFrequency==0)
	{
		uRef2 = uRef1;
		uRef1 -= gridFrequency;
		if (uRef1 < 0)
		{
			uRef1 = 4 * gridFrequency;
		}
	}
	int32 vRef2 = (vLoc / gridFrequency)*gridFrequency;
	int32 vRef1 = (vLoc / gridFrequency)*gridFrequency+gridFrequency;

	int32 localU = uLoc - uRef1;
	int32 localV = vLoc - vRef2;
	FVector refPoint = gridReferencePointsM[gridLocationsM[uRef1][vRef1]];

	FVector uDir = gridReferencePointsM[gridLocationsM[uRef2][vRef2]] - refPoint;
	uDir /= gridFrequency;
	FVector vDir;
	if (localV >= localU)
	{
		//use the "upper vRef location
		vDir = gridReferencePointsM[gridLocationsM[uRef1][vRef2]] - refPoint;
	}
	else
	{
		//use the lower vRef location
		vDir = gridReferencePointsM[gridLocationsM[uRef2][vRef1]] - refPoint;
	}
	vDir /= gridFrequency;

	//position on icosahedron
	FVector icosahedronPos = refPoint + localU*uDir + (localU - localV)*vDir;

	FVector unitSphereVec;
	float currentMag;
	icosahedronPos.ToDirectionAndLength(unitSphereVec, currentMag);
	return unitSphereVec;
}

TArray<FRectGridLocation> USphereGrid::getTileNeighbors(const FRectGridLocation& gridTile) const
{
	return getTilesNStepsAway(gridTile, 1);
}

TArray<FRectGridLocation> USphereGrid::getTilesNStepsAway(const FRectGridLocation& gridTile, const int32& numSteps) const
{
	TArray<FRectGridLocation> tilesInRange;
	//our Manhattan distance plane is u-v+w=0
	int32 startU = gridPositions[0].uPos;
	int32 startV = gridPositions[0].uPos;

	//do each side in turn
	//u = numSteps
	int32 offsetU = numSteps;
	int32 offsetV = 0;
	for (; offsetV < numSteps; ++offsetV)
	{
		tilesInRange.AddUnique(getOffsetTile(startU, startV, offsetU, offsetV));
	}
	//v = numSteps1
	for (; offsetU > 0; --offsetU)
	{
		tilesInRange.AddUnique(getOffsetTile(startU, startV, offsetU, offsetV));
	}
	//w = numSteps1
	for (; offsetV > 0; --offsetU, --offsetV)
	{
		tilesInRange.AddUnique(getOffsetTile(startU, startV, offsetU, offsetV));
	}
	//u = -numSteps1
	for (; offsetV > -numSteps; --offsetV)
	{
		tilesInRange.AddUnique(getOffsetTile(startU, startV, offsetU, offsetV));
	}
	//v = -numSteps
	for (; offsetU < 0; ++offsetU)
	{
		tilesInRange.AddUnique(getOffsetTile(startU, startV, offsetU, offsetV));
	}
	//w = -numSteps
	for (; offsetV < 0; ++offsetU, ++offsetV)
	{
		tilesInRange.AddUnique(getOffsetTile(startU, startV, offsetU, offsetV));
	}
	return tilesInRange;
}

TArray<FRectGridLocation> USphereGrid::getStraightPathBetweenTiles(const FRectGridLocation& startTile, const FRectGridLocation& startTile) const
{

}

