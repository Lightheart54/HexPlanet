// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "TectonicPlateSimulator.h"
#include "SimplexNoiseBPLibrary.h"


// Sets default values for this component's properties
UTectonicPlateSimulator::UTectonicPlateSimulator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	myMesher = nullptr; 
	myGrid = nullptr;
	plateSeed = FMath::Rand();
	numBasePlates = 12;
	numBaseSubplates = 4;
	addSubplatesAfterNSteps = 6;
	percentTilesForShapeReseed = 0.05;
	percentTilesForBorderReseed = 0.75;
	showPlateOverlay = false;
	stopAfterFirstPlate = false; 
	plateToShowCenterOfMassDebugPoints = -1;
	overlayMaterial = nullptr;
	heightMapSeed = FMath::Rand();
	numOctaves = 1; 
	percentOcean = 60;
	percentContinentalCrust = 65;
	showInitialContinents = false;

	lithosphereDensity = 3400;
	oceanicCrustDensity = 3000;
	continentalCrustDensity = 2600;
	oceanicWaterDensity = 1025;
	elevationColorKey.Add(FColor::Black);
	elevationColorKey.Add(FColor::Blue);
	elevationColorKey.Add(FColor::Green);
	elevationColorKey.Add(FColor::White);
	// ...
}


// Called when the game starts
void UTectonicPlateSimulator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTectonicPlateSimulator::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

void UTectonicPlateSimulator::generateInitialHeightMap()
{
	//use 3d simplex noise to generate a continuous random starting height map
	//for our sphere
	TArray<float> initialHeightMap;
	initialHeightMap.SetNumZeroed(myGrid->numNodes);
	USimplexNoiseBPLibrary::setNoiseSeed(heightMapSeed);
	float elevationStep = 4.0 / (elevationColorKey.Num());
	TArray<FColor> indexColors;
	indexColors.SetNumUninitialized(myGrid->numNodes);
	for (int32 nodeIndex = 0; nodeIndex < myGrid->numNodes;++nodeIndex)
	{
		FVector nodeLocation = myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[nodeIndex]);
		float magReduction = 1;
		for (int32 octave = 0; octave < numOctaves;++octave)
		{
			initialHeightMap[nodeIndex] += USimplexNoiseBPLibrary::SimplexNoise3D(nodeLocation.X, nodeLocation.Y, nodeLocation.Z)/magReduction;
			nodeLocation *= 2;
			magReduction *= 2;
		}
		initialHeightMap[nodeIndex] += 2; //normalize to be zero based
		int32 colorChoice = FMath::FloorToInt((initialHeightMap[nodeIndex]) / elevationStep);
		if (colorChoice >= myGrid->numNodes)
		{
			colorChoice = myGrid->numNodes - 1;
		}
		indexColors[nodeIndex] = elevationColorKey[colorChoice];
	}

	if (showBaseHeightMap)
	{
		TArray<float> overlayRadii;
		overlayRadii.Init(myMesher->baseMeshRadius, myGrid->numNodes);
		myMesher->buildNewMesh(overlayRadii, indexColors, overlayMaterial);
	}

	//create a distribution of the height map in order to separate between oceanic crust and 
	//continental crust
	TArray<float> heightToSort(initialHeightMap);
	heightToSort.Sort();
	int32 oceanDepthIndex = FMath::FloorToInt(heightToSort.Num()*percentOcean / 100.0);
	int32 continentalCrustIndex = FMath::FloorToInt(heightToSort.Num()*(100-percentContinentalCrust) / 100.0);
	float minHeight = heightToSort[0];
	float maxHeight = heightToSort.Last();
	float baseContinentalHeight = heightToSort[continentalCrustIndex];
	float baseOceanDepth = heightToSort[oceanDepthIndex];
	float oceanicCrustFactor = 0.1;
	float continentalCrustFactor = 1.0;
	TArray<FColor> continentKeyColor;
	continentKeyColor.SetNumZeroed(myGrid->numNodes);
	crustCells.SetNumZeroed(myGrid->numNodes);
	//normalize and separate into oceanic crust and continental crust
	for (int32 nodeIndex = 0; nodeIndex < myGrid->numNodes; ++nodeIndex)
	{
		if (initialHeightMap[nodeIndex] >= baseContinentalHeight)
		{
			initialHeightMap[nodeIndex] *= continentalCrustFactor / baseOceanDepth;
			if (initialHeightMap[nodeIndex] >= 1.0)
			{
				continentKeyColor[nodeIndex] = FColor::Green;
			}
			else
			{
				continentKeyColor[nodeIndex] = FColor(0, 255, 255);
			}
		}
		else
		{
			initialHeightMap[nodeIndex] *= oceanicCrustFactor / baseOceanDepth;
			continentKeyColor[nodeIndex] = FColor::Blue;
		}
		crustCells[nodeIndex] = createBaseCrustCell(nodeIndex, initialHeightMap[nodeIndex]);
	}
	if (showInitialContinents)
	{
		TArray<float> overlayRadii;
		overlayRadii.Init(myMesher->baseMeshRadius, myGrid->numNodes);
		myMesher->buildNewMesh(overlayRadii, continentKeyColor, overlayMaterial);
	}
}

FCrustCellData UTectonicPlateSimulator::createBaseCrustCell(const int32& cellIndex, const float& cellHeight) const
{
	//water height is 1.0;
	FCrustCellData newCellData;
	newCellData.gridLoc = myGrid->gridLocationsM[cellIndex];
	newCellData.cellHeight = cellHeight;
	newCellData.owningPlate = -1;
	//get the cell area
	//approximately 4*pi*radius^2/numNodes
	newCellData.crustArea = 4 * PI*FMath::Pow(myMesher->baseMeshRadius, 2) / myGrid->numNodes;
	float crustMass = continentalCrustDensity*newCellData.crustArea;

	if (cellHeight < 1.0)
	{
		float lowerThickness = ((1 - cellHeight)*oceanicWaterDensity + cellHeight*oceanicCrustDensity) / (lithosphereDensity - oceanicCrustDensity);
		newCellData.crustThickness = cellHeight + lowerThickness;
		newCellData.crustDensity = oceanicCrustDensity;
	}
	else
	{
		float lowerThickness = cellHeight*continentalCrustDensity / (lithosphereDensity - continentalCrustDensity);
		newCellData.crustThickness = cellHeight + lowerThickness;
		newCellData.crustDensity = continentalCrustDensity;
	}
	return newCellData;
}

void UTectonicPlateSimulator::buildTectonicPlates()
{
	TArray<TArray<int32>> currentPlateSets;
	currentPlateSets.Empty();
	TArray<bool> usedTiles;
	FMath::RandInit(plateSeed);
	usedTiles.Init(true, myGrid->numNodes);
	addNewSeedSetsToSetArray(usedTiles, currentPlateSets,numBasePlates);
	createVoronoiDiagramFromSeedSets(currentPlateSets,usedTiles, addSubplatesAfterNSteps);
	addNewSeedSetsToSetArray(usedTiles, currentPlateSets, numBaseSubplates);
	createVoronoiDiagramFromSeedSets(currentPlateSets, usedTiles);

	// rebuild the plates from a random set of seed tiles inside of the plate
	// to adjust the overall shape of the plate
	rebuildTectonicPlates(currentPlateSets, percentTilesForShapeReseed);
	// rebuild the plates from a random set of seed tiles inside of the plate
	// to adjust the shape of plate borders
	rebuildTectonicPlates(currentPlateSets, percentTilesForBorderReseed);

	currentPlates.Empty();
	currentPlates.SetNumZeroed(currentPlateSets.Num());
	for (int32 plateIndex = 0; plateIndex < currentPlateSets.Num();++plateIndex)
	{
		currentPlates[plateIndex] = createTectonicPlate(plateIndex, currentPlateSets[plateIndex]);
		if (stopAfterFirstPlate)
		{
			break;
		}
	}

	if (showPlateOverlay)
	{
		meshTectonicPlateOverlay();
	}
}

void UTectonicPlateSimulator::addNewSeedSetsToSetArray(TArray<bool> &usedTiles, TArray<TArray<int32>> &plateSets, const int32& numNewSets)
{
	for (int it = 0; it < numNewSets; ++it)
	{
		TArray<int32> newSet;
		int32 seedTile;
		seedTile = getNextAvailableSeedTile(usedTiles, plateSets);
		if (seedTile != -1)
		{
			newSet.Add(seedTile);
			usedTiles[seedTile] = false;
			plateSets.Add(newSet);
		}
		else
		{
			break;
		}
	}
}

int32 UTectonicPlateSimulator::getNextAvailableSeedTile(TArray<bool> &usedTiles, TArray<TArray<int32>> & plateSets)
{
	bool success = false;
	int32 seedTile;
	while (!success)
	{
		seedTile = FMath::RandRange(0, myGrid->numNodes - 1);
		if (usedTiles[seedTile])
		{
			return seedTile;
		}
		else
		{
			int32 numUsedTiles = 0;
			for (const TArray<int32>& existingSet : plateSets)
			{
				numUsedTiles += existingSet.Num();
			}
			if (numUsedTiles >= myGrid->numNodes)
			{
				break;
			}
		}
	}
	return -1;
}

void UTectonicPlateSimulator::createVoronoiDiagramFromSeedSets(TArray<TArray<int32>>& seedSets,
	TArray<bool>& tileAvailability, const int32& maxNumIterations /*= -1*/)
{
	//produce our voronoi diagram using Manhattan distances
	int32 currentIt = 0;
	bool tryAgain = true;
	while (tryAgain)
	{
		tryAgain = false;
		for (TArray<int32>& tileSet : seedSets)
		{
			int32 startNumTiles = tileSet.Num();
			myGrid->expandTileSet(tileSet, tileAvailability);
			tryAgain |= startNumTiles < tileSet.Num();
		}
		if (maxNumIterations == currentIt)
		{
			break;
		}
		currentIt++;
	}
}

void UTectonicPlateSimulator::rebuildTectonicPlates(TArray<TArray<int32>>& plateSets, const float& percentTilesForReseed)
{
	TArray<bool> usedTiles;
	usedTiles.Init(true, myGrid->numNodes);
	for (TArray<int32>& tileSet : plateSets)
	{
		int32 numSubSeeds = FMath::RoundToInt(tileSet.Num() * percentTilesForReseed) + 1;
		TArray<int32> subSeedNums;
		for (int32 subseedNum = 0; subseedNum < numSubSeeds; ++subseedNum)
		{
			int32 newSubSeed = FMath::RandRange(0, tileSet.Num() - 1);
			subSeedNums.Add(tileSet[newSubSeed]);
		}
		tileSet.Empty();
		for (const int32& subSeed : subSeedNums)
		{
			tileSet.Add(subSeed);
			usedTiles[subSeed] = false;
		}
	}
	createVoronoiDiagramFromSeedSets(plateSets,usedTiles);
}

void UTectonicPlateSimulator::meshTectonicPlateOverlay()
{
	TArray<FColor> vertexColors;
	TArray<float> vertexRadii;
	vertexColors.SetNumZeroed(myGrid->numNodes);
	vertexRadii.SetNumZeroed(myGrid->numNodes);
	for (const FTectonicPlate& tectonicPlate : currentPlates)
	{
		//make new color
		uint8 rValue = FMath::RandRange(0.0, 255);
		uint8 gValue = FMath::FRandRange(0.0, 255);
		uint8 bValue = FMath::FRandRange(0.0, 255);
		FColor plateColor(rValue, gValue, bValue);
		for (const int32& nodeIndex : tectonicPlate.ownedCrustCells)
		{
			vertexColors[nodeIndex] = plateColor;
			vertexRadii[nodeIndex] = myMesher->baseMeshRadius;
		}
		myMesher->debugLineOut->DrawPoint(myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[tectonicPlate.centerOfMassIndex])
			* myMesher->baseMeshRadius*1.1, plateColor, 10, 2);
		if (stopAfterFirstPlate)
		{
			break;
		}
	}
	myMesher->buildNewMesh(vertexRadii, vertexColors, overlayMaterial);
}

FTectonicPlate UTectonicPlateSimulator::createTectonicPlate(const int32& plateIndex, const TArray<int32>& plateCellIndexes) const
{
	FTectonicPlate newPlate;
	newPlate.plateIndex = plateIndex;
	newPlate.ownedCrustCells = plateCellIndexes;
	newPlate.currentVelocity = FVector(0, 0, 0);
	updatePlateCenterOfMass(newPlate);
	updatePlateBoundingRadius(newPlate);	
	return newPlate;
}

void UTectonicPlateSimulator::updatePlateCenterOfMass(FTectonicPlate &newPlate) const
{
	if (newPlate.ownedCrustCells.Num()==0)
	{
		newPlate.centerOfMassIndex = -1;
	}
	else
	{
		FVector massMomentArm(0, 0, 0);
		float totalMass = 0.0;
		for (const int32& plateCell : newPlate.ownedCrustCells)
		{
			float cellMass = crustCells[plateCell].crustThickness*crustCells[plateCell].crustArea*crustCells[plateCell].crustDensity;
			totalMass += cellMass;
			massMomentArm += cellMass * myGrid->getNodeLocationOnSphere(crustCells[plateCell].gridLoc)
				*(myMesher->baseMeshRadius + crustCells[plateCell].cellHeight - crustCells[plateCell].crustThickness / 2);
		}
		FVector centerOfMass = massMomentArm / totalMass;
		if (showPlateOverlay && plateToShowCenterOfMassDebugPoints == newPlate.plateIndex)
		{
			myMesher->debugLineOut->DrawPoint(centerOfMass * 1.05*myMesher->baseMeshRadius / FMath::Sqrt(FVector::DotProduct(centerOfMass, centerOfMass)),
				FLinearColor::Blue, 10, 2);
			newPlate.centerOfMassIndex = myGrid->mapPosToTileIndex(centerOfMass, myMesher->debugLineOut, myMesher->baseMeshRadius);
		}
		else
		{
			newPlate.centerOfMassIndex = myGrid->mapPosToTileIndex(centerOfMass);
		}
	}
}

void UTectonicPlateSimulator::updatePlateBoundingRadius(FTectonicPlate& newPlate) const
{
	float boundingRadius = 0.0;
	FVector plateCenterDir = myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[newPlate.centerOfMassIndex]);
	for (const int32& plateCell : newPlate.ownedCrustCells)
	{
		FVector cellCenter = myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[plateCell]);
		float cellArcDistance = FMath::Acos(FVector::DotProduct(plateCenterDir, cellCenter));
		newPlate.plateBoundingRadius = FMath::Max(cellArcDistance, newPlate.plateBoundingRadius);
	}
}

