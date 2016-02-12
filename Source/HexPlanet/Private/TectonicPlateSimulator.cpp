// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "TectonicPlateSimulator.h"


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
	overlayMaterial = nullptr;

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

void UTectonicPlateSimulator::buildTectonicPlates()
{
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
	rebuildTectonicPlate(currentPlateSets, percentTilesForShapeReseed);
	// rebuild the plates from a random set of seed tiles inside of the plate
	// to adjust the shape of plate borders
	rebuildTectonicPlate(currentPlateSets, percentTilesForBorderReseed);

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

void UTectonicPlateSimulator::rebuildTectonicPlate(TArray<TArray<int32>>& plateSets, const float& percentTilesForReseed)
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
	for (const TArray<int32> & plateSet : currentPlateSets)
	{
		//make new color
		uint8 rValue = FMath::RandRange(0.0, 255);
		uint8 gValue = FMath::FRandRange(0.0, 255);
		uint8 bValue = FMath::FRandRange(0.0, 255);
		FColor plateColor(rValue, gValue, bValue);
		for (const int32& nodeIndex : plateSet)
		{
			vertexColors[nodeIndex] = plateColor;
			vertexRadii[nodeIndex] = myMesher->baseMeshRadius;
		}
	}
	myMesher->buildNewMesh(vertexRadii, vertexColors, overlayMaterial);
}

