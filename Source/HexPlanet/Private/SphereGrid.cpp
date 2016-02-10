// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "SphereGrid.h"
#include <limits>
#include <cassert>


// Sets default values for this component's properties
USphereGrid::USphereGrid()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;
	gridFrequency = 1;
	numNodes = 2 + 10 * FMath::Pow(3,gridFrequency-1);
	icosahedronInteriorAngle = 0;
	// ...
}


// Called when the game starts
void USphereGrid::BeginPlay()
{
	Super::BeginPlay();

	//creating base icosahedron
	TArray<FVector> nodeLocations = createBaseIcosahedron();

	// setup grid
	numNodes = 2 + 10 * FMath::Pow(3,gridFrequency-1);
	gridLocationsM.SetNumZeroed(numNodes);
	rectilinearGridM.SetNum(5*gridFrequency);
	int32 tileNum = 0;
	for (int32 uLoc = 0; uLoc < rectilinearGridM.Num(); ++uLoc)
	{
		int32 vSize = 2 * gridFrequency + 1 + (uLoc % gridFrequency == 0 ? gridFrequency : 0);
		rectilinearGridM[uLoc].SetNumZeroed(vSize);
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
					rectilinearGridM[uLoc][vLoc] = rectilinearGridM[0][0];
				}
				else if (uLoc % gridFrequency != 0)
				{
					rectilinearGridM[uLoc][vLoc] = rectilinearGridM[uLoc / gridFrequency][uLoc%gridFrequency];
				}
			}
			else if (vLoc == vSize-1 && uLoc != 0)
			{
				if (uLoc % gridFrequency == 0)
				{
					rectilinearGridM[uLoc][vLoc] = rectilinearGridM[0][vLoc];
				}
				else if (uLoc % gridFrequency != 0)
				{
					rectilinearGridM[uLoc][vLoc] = rectilinearGridM[uLoc - vLoc % gridFrequency][(vLoc/gridFrequency) * gridFrequency];
				}
			}
			else
			{
				rectilinearGridM[uLoc][vLoc] = tileNum;
				gridLocationsM[rectilinearGridM[uLoc][vLoc]].tileIndex = tileNum;
				++tileNum;
			}
			gridLocationsM[rectilinearGridM[uLoc][vLoc]].gridPositions.Add(newIndex);

			//add this point as a reference point if it doesn't already exist
			if (uLoc%gridFrequency==0 && vLoc%gridFrequency ==0)
			{
				if (!gridReferencePointsM.Contains(rectilinearGridM[uLoc][vLoc]))
				{
					//order needs to be 1,8,4,2,6,9,3,11,7,5,10,0
					switch (gridReferencePointsM.Num())
					{
					case 0:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[1]);
						break;
					case 1:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[8]);
						break;
					case 2:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[4]);
						break;
					case 3:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[2]);
						break;
					case 4:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[6]);
						break;
					case 5:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[9]);
						break;
					case 6:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[3]);
						break;
					case 7:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[11]);
						break;
					case 8:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[7]);
						break;
					case 9:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[5]);
						break;
					case 10:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[10]);
						break;
					case 11:
						gridReferencePointsM.Add(rectilinearGridM[uLoc][vLoc], nodeLocations[0]);
						break;
					}
				}
			}
		}
	}
}

TArray<FVector> USphereGrid::createBaseIcosahedron()
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

	icosahedronInteriorAngle = FMath::Acos(FVector::DotProduct(nodeLocations[0], nodeLocations[1]));

	return nodeLocations;
}

// Called every frame
void USphereGrid::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

FRectGridLocation USphereGrid::mapPosToTile(const FVector& positionOnSphere) const
{
	return gridLocationsM[mapPosToTileIndex(positionOnSphere)];
}

int32 USphereGrid::mapPosToTileIndex(const FVector& positionOnSphere) const
{
	//find the four closest reference points, this establishes the icsoahedron face we're on
	TArray<int32> refenceIndexes;
	gridReferencePointsM.GetKeys(refenceIndexes);
	refenceIndexes.Sort([&](const int32& pos1, const int32& pos2)->bool
	{
		return FVector::Dist(positionOnSphere, gridReferencePointsM[pos1]) < FVector::Dist(positionOnSphere, gridReferencePointsM[pos2]);
	});

	TArray<int32> refPoints;
	refPoints.Add(refenceIndexes[0]);
	refPoints.Add(refenceIndexes[1]);
	refPoints.Add(refenceIndexes[2]);
	refPoints.Add(refenceIndexes[3]);

	//look at the uPositions and establish the reference square
	int32 uRef1 = std::numeric_limits<int32>::max();
	int32 uRef2 = std::numeric_limits<int32>::max();
	int32 vRef11 = std::numeric_limits<int32>::max();
	int32 vRef12 = std::numeric_limits<int32>::max();
	int32 vRef21 = std::numeric_limits<int32>::max();
	int32 vRef22 = std::numeric_limits<int32>::max();
	for (const int32& index : refPoints)
	{
		if (gridLocationsM[index].gridPositions.Num() == 0)
		{
			FRectGridIndex refIndex = gridLocationsM[index].gridPositions[0];
			if (refIndex.uPos <= uRef1)
			{
				uRef1 = refIndex.uPos;
				if (refIndex.vPos < vRef11)
				{
					vRef12 = vRef11;
					vRef11 = refIndex.vPos;
				}
				else
				{
					vRef12 = refIndex.vPos;
				}
			}
			else
			{
				uRef2 = refIndex.uPos;
				if (refIndex.vPos < vRef21)
				{
					vRef22 = vRef21;
					vRef21 = refIndex.vPos;
				}
				else
				{
					vRef22 = refIndex.vPos;
				}
			}

		}
	}
	//finally switch them if uref1 == 0
	if (uRef1 == 0)
	{
		int32 tmpVal;
		tmpVal = uRef1;
		uRef1 = uRef2;
		uRef2 = tmpVal;
		tmpVal = vRef11;
		vRef11 = vRef21;
		vRef21 = tmpVal;
		tmpVal = vRef12;
		vRef12 = vRef22;
		vRef22 = tmpVal;
	}
	//we can now establish the rest based upon which vrefs have been set;
	if (vRef12 == std::numeric_limits<int32>::max())
	{
		vRef12 = 3 * gridFrequency;
	}
	else if (vRef22 == std::numeric_limits<int32>::max())
	{
		vRef22 = vRef21;
		vRef21 = 0;
	}

	//establish the uDir
	FVector refPoint = gridReferencePointsM[rectilinearGridM[uRef1][vRef11]];

	FVector uDir = gridReferencePointsM[rectilinearGridM[uRef2][vRef22]] - refPoint;
	uDir /= gridFrequency;

	FVector vDir;

	//if we're closer to u1v12 than u2v21 we're in the upper triangle, otherwise we're in the lower triangle
	if (refPoints.Find(rectilinearGridM[uRef1][vRef12]) < refPoints.Find(rectilinearGridM[uRef2][vRef21]))
	{
		vDir = gridReferencePointsM[rectilinearGridM[uRef1][vRef12]] - gridReferencePointsM[rectilinearGridM[uRef1][vRef11]];
	}
	else
	{
		vDir = gridReferencePointsM[rectilinearGridM[uRef2][vRef22]] - gridReferencePointsM[rectilinearGridM[uRef2][vRef21]];
	}

	//the local Vector
	FVector projectedVector = projectVectorOntoIcosahedronFace(positionOnSphere, refPoint, uDir, vDir);
	FVector localVector = projectedVector - refPoint;

	//determine u
	float uIncAprox = FVector::DotProduct(localVector, uDir);
	int32 uInc = FMath::RoundToInt(uIncAprox);

	FVector localV = localVector - uIncAprox*uDir;
	float vIncAprox = FVector::DotProduct(localV, vDir);
	int32 vInc = FMath::RoundToInt(vIncAprox);

	return rectilinearGridM[uRef1 + uInc][vRef11 + uInc + vInc];
}

FVector USphereGrid::getNodeLocationOnSphere(const int32& uLoc, const int32& vLoc) const
{
	//find the reference vectors
	int32 uRef1 = (uLoc / gridFrequency)*gridFrequency;
	int32 uRef2 = uRef1 + gridFrequency;

	//make sure we're mapping to a real triangle
	if (uRef1 % gridFrequency == 0
		&& vLoc < gridFrequency)
	{
		uRef1 -= gridFrequency;
		uRef2 -= gridFrequency;
	}
	//adjust for warp around
	if (uRef2 >= 5 * gridFrequency)
	{
		uRef2 = gridFrequency;
	}
	if (uRef1 < 0)
	{
		uRef1 = 4 * gridFrequency;
	}


	int32 vRef11 = int32((vLoc / gridFrequency)*gridFrequency) + int32((uLoc==uRef1)?0:gridFrequency);
	if (vLoc == rectilinearGridM[uLoc].Num()-1)
	{
		vRef11 -= gridFrequency;
	}
	int32 vRef12 = vRef11 + gridFrequency;

	int32 vRef21 = vRef11 - gridFrequency;
	int32 vRef22 = vRef12 - gridFrequency;


	int32 localU = (uLoc==0 && vLoc<gridFrequency)? gridFrequency:uLoc - uRef1;
	int32 localV = vLoc - int32(localU==0?vRef11:vRef21);
	FVector refPoint = gridReferencePointsM[rectilinearGridM[uRef1][vRef11]];

	FVector uDir = gridReferencePointsM[rectilinearGridM[uRef2][vRef22]] - refPoint;
	uDir /= gridFrequency;
	FVector vDir;
	if (localV >= localU)
	{
		//use the "upper vRef location
		vDir = gridReferencePointsM[rectilinearGridM[uRef1][vRef12]] - gridReferencePointsM[rectilinearGridM[uRef1][vRef11]];
	}
	else
	{
		//use the lower vRef location
		vDir = gridReferencePointsM[rectilinearGridM[uRef2][vRef22]] - gridReferencePointsM[rectilinearGridM[uRef2][vRef21]];
	}
	vDir /= gridFrequency;

	//position on icosahedron
	FVector icosahedronPos = refPoint + localU*uDir + (localV - localU)*vDir;

	FVector unitSphereVec;
	float currentMag;
	icosahedronPos.ToDirectionAndLength(unitSphereVec, currentMag);
	return unitSphereVec;
}

TArray<FRectGridLocation> USphereGrid::getLocationsForIndexes(const TArray<int32>& locationIndexs) const
{
	TArray<FRectGridLocation> neighbors;
	for (const int32& indexNeighbor : locationIndexs)
	{
		neighbors.Add(gridLocationsM[indexNeighbor]);
	}
	return neighbors;
}

void USphereGrid::addTileToNeighborList(int32 nextU, int32 nextV, TArray<int32> &tilesInRange, int32& nextTileIndex) const
{
	int32 nextTile = rectilinearGridM[nextU][nextV];
	int32 newLocation = 0;
	if (!tilesInRange.Find(nextTile, newLocation))
	{
		newLocation = tilesInRange.Insert(nextTile, nextTileIndex);
	}
	nextTileIndex = newLocation;
}

TArray<FRectGridLocation> USphereGrid::getTileNeighbors(const FRectGridLocation& gridTile) const
{
	TArray<int32> neighborList = getTileNeighborIndexes(gridTile);
	return getLocationsForIndexes(neighborList);
}

TArray<int32> USphereGrid::getTileNeighborIndexes(const FRectGridLocation& gridTile) const
{
	TArray<int32> neighborList;

	for (const FRectGridIndex& gridIndex : gridTile.gridPositions)
	{
		TArray<int32> tilesInRange = getIndexNeighbors(gridIndex);
		//combine the tilesInRangeList with the neighborList such that the order is maintained
		//start by finding a shared neighbor
		int32 startLoc = 0;
		int32 tIRIndex = 0;
		int32 numCornersToTransfer = 5;
		if (neighborList.Num()!=0)
		{
			startLoc = std::numeric_limits<int32>::min();
			for (; tIRIndex < tilesInRange.Num(); ++tIRIndex)
			{
				int32 tileIndex = tilesInRange[tIRIndex];
				if (tileIndex != std::numeric_limits<int32>::min())
				{
					startLoc = neighborList.Find(tileIndex);
					if (startLoc >= 0)
					{
						break;
					}
				}
			}
			++startLoc;
			++tIRIndex;
		}
		else
		{
			neighborList.SetNumUninitialized(6);
			for (int32 i = 0; i < 6; ++i)
			{
				neighborList[i] = std::numeric_limits<int32>::min();
			}
			numCornersToTransfer = 6;
		}
		for (int numCorners = 0; numCorners < numCornersToTransfer; ++numCorners, ++startLoc, ++tIRIndex)
		{
			if (tIRIndex == 6)
			{
				tIRIndex = 0;
			}
			if (startLoc == 6)
			{
				startLoc = 0;
			}
			if (tilesInRange[tIRIndex] != std::numeric_limits<int32>::min())
			{
				if (neighborList.Contains(tilesInRange[tIRIndex]))
				{
					continue;
				}
				neighborList[startLoc] = tilesInRange[tIRIndex];
			}
		}
	}
	neighborList.Remove(std::numeric_limits<int32>::min());
	return neighborList;
}

TArray<int32> USphereGrid::getIndexNeighbors(const FRectGridIndex &gridIndex) const
{
	int32 nextU = gridIndex.uPos;
	int32 nextV = gridIndex.vPos;
	int32 myIndex = rectilinearGridM[nextU][nextV];
	TArray<int32> tilesInRange;
	tilesInRange.SetNumUninitialized(6);
	for (int32 i = 0; i < 6; i++)
	{
		tilesInRange[i] = std::numeric_limits<int32>::min();
	}
	//our Manhattan distance plane is u-v+w=0
	//u1,v0
	incrementU(nextU, nextV);

	if (nextV >= 0)
	{
		tilesInRange[0] = (rectilinearGridM[nextU][nextV]);
		if (tilesInRange[0] == myIndex)
		{
			tilesInRange[0] = std::numeric_limits<int32>::min();
		}

	}
	//u1,v1
	++nextV;
	if (nextV >= 0 && nextV < rectilinearGridM[nextU].Num())
	{
		tilesInRange[1] = (rectilinearGridM[nextU][nextV]);
		if (tilesInRange[1] == myIndex)
		{
			tilesInRange[1] = std::numeric_limits<int32>::min();
		}
	}
	//u0,v1
	decrementU(nextU, nextV);

	if (nextV < rectilinearGridM[nextU].Num())
	{
		tilesInRange[2] = (rectilinearGridM[nextU][nextV]);
		if (tilesInRange[2] == myIndex)
		{
			tilesInRange[2] = std::numeric_limits<int32>::min();
		}
	}
	//u-1,v0
	decrementU(nextU, nextV);
	--nextV;
	if (nextV < rectilinearGridM[nextU].Num())
	{
		tilesInRange[3] = (rectilinearGridM[nextU][nextV]);
		if (tilesInRange[3] == myIndex)
		{
			tilesInRange[3] = std::numeric_limits<int32>::min();
		}
	}
	//u-1,v-1
	--nextV;
	if (nextV >= 0 && nextV < rectilinearGridM[nextU].Num())
	{
		tilesInRange[4] = (rectilinearGridM[nextU][nextV]);
		if (tilesInRange[4] == myIndex)
		{
			tilesInRange[4] = std::numeric_limits<int32>::min();
		}
	}
	//u0,v-1
	incrementU(nextU, nextV);
	if (nextV >= 0)
	{
		tilesInRange[5] = (rectilinearGridM[nextU][nextV]);
		if (tilesInRange[5] == myIndex)
		{
			tilesInRange[5] = std::numeric_limits<int32>::min();
		}
	}
	return tilesInRange;
}

void USphereGrid::decrementU(int32 &nextU, int32 &nextV) const
{
	--nextU;
	if (nextU%gridFrequency == 0)
	{
		nextV += gridFrequency;
	}
	if (nextU < 0)
	{
		nextU += gridFrequency * 5;
	}
}

void USphereGrid::incrementU(int32 &nextU, int32 &nextV) const
{
	int32 oldPhase = FMath::CeilToInt(nextU*1.0 / gridFrequency);
	++nextU;
	if (FMath::CeilToInt(nextU*1.0 / gridFrequency) > oldPhase)
	{
		nextV -= gridFrequency;
	}
	if (nextU == gridFrequency * 5)
	{
		nextU = 0;
	}
}

TArray<FRectGridLocation> USphereGrid::getTilesNStepsAway(const FRectGridLocation& gridTile, const int32& numSteps) const
{
	TArray<int32> tileIndexes = getTileIndexesNStepsAway(gridTile, numSteps);
	return getLocationsForIndexes(tileIndexes);
}

TArray<int32> USphereGrid::getTileIndexesNStepsAway(const FRectGridLocation& gridTile, const int32& numSteps) const
{
	TSet<int32> tileIndexSet;
	tileIndexSet.Add(gridTile.tileIndex);
	for (int32 step = 0; step < numSteps; ++step)
	{
		expandTileSet(tileIndexSet);
	}
	return tileIndexSet.Array();
}

void USphereGrid::expandTileSet(TSet<int32>& tileIndexSet) const
{
	TSet<int32> expandedSet;
	for (const int32& tileNum : tileIndexSet)
	{
		for (const FRectGridIndex& gridIndex : gridLocationsM[tileNum].gridPositions)
		{
			TSet<int32> indexSet(getIndexNeighbors(gridIndex));
			expandedSet.Union(indexSet);
		}
	}
	expandedSet.Remove(std::numeric_limits<int32>::min());
	tileIndexSet = expandedSet;
}

FVector USphereGrid::projectVectorOntoIcosahedronFace(const FVector& positionOnSphere, const FVector& refPoint, const FVector& uDir, const FVector& vDir) const
{
	FVector planeVec = FVector::CrossProduct(uDir, vDir);
	planeVec /= FVector::DotProduct(planeVec, planeVec);
	float planeR = FVector::DotProduct(planeVec, refPoint);
	if (planeR < 0)
	{
		planeVec *= -1;
		planeR *= -1;
	}
	return planeR*positionOnSphere / FVector::DotProduct(planeVec, positionOnSphere);
}

TArray<FRectGridLocation> USphereGrid::getStraightPathBetweenTiles(const FRectGridLocation& startTile, const FRectGridLocation& endTile) const
{
	return getLocationsForIndexes(getStraightIndexPathBetweenTiles(startTile, endTile));
}

TArray<int32> USphereGrid::getStraightIndexPathBetweenTiles(const FRectGridLocation& startTile, const FRectGridLocation& endTile) const
{
	//draw the great circle

	//split the arc between the two points into sections length icosahedronInterriorAngle/gridFrequency

	//at every set get the grid index
	return TArray<int32>();
}

