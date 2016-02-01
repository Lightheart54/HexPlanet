// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "ContinentGenerator.h"


// Sets default values for this component's properties
UContinentGenerator::UContinentGenerator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	//tectonic plate settings
	tectonicPlateSeed = FMath::Rand();
	numberOfPlateSeeds = 12;
	numberOfSubPlateSeeds = 2;
	addSubPlatesAfterIteration = 2;

	//land massing settings
	landMassingSeed = FMath::Rand();
	percentOcean = .6;
	minPrimaryPlateTypeSeeds = 1;
	primaryPlateTypeDensity = 3;
	minSecondaryPlateTypeSeeds = 4;
	secondaryPlateTypeDensity = 0;
	hexagonLandInstanceMesher = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HexagonLandMesher"));
	hexagonOceanInstanceMesher = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HexagonOceanMesher"));
	pentagonLandInstanceMesher = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PentagonLandMesher"));
	pentagonOceanInstanceMesher = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PentagonOceanMesher"));
	overlayLandWaterMap = false;
}


// Called when the game starts
void UContinentGenerator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UContinentGenerator::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

void UContinentGenerator::installGrid(AHexSphere* myOwner, GridGenerator* gridGenerator)
{
	gridOwner = myOwner;
	gridGen = gridGenerator;
	hexagonLandInstanceMesher->AttachTo(myOwner->GetRootComponent());
	pentagonLandInstanceMesher->AttachTo(myOwner->GetRootComponent());
	hexagonOceanInstanceMesher->AttachTo(myOwner->GetRootComponent());
	pentagonOceanInstanceMesher->AttachTo(myOwner->GetRootComponent());
}

void UContinentGenerator::buildTectonicPlates()
{
	TArray<FGridTileSet> plateTileSets;
	FRandomStream randStream(tectonicPlateSeed);
	plateTileSets.SetNumZeroed(numberOfPlateSeeds);
	GridTilePtrList gridTiles = gridGen->getTiles();
	for (int it = 0; it < numberOfPlateSeeds; ++it)
	{
		FGridTileSet& tileSet = plateTileSets[it];
		tileSet.setIndex = it;
		int32 seedTile;
		do 
		{
			seedTile = randStream.RandRange(0, gridOwner->numTiles - 1);
		} while (!addTileToTileSet(tileSet, seedTile, gridTiles));
		tileSet.setTags.Add(uint8(EPlateTypeEnum::PT_Unassigned));
	}

	createVoronoiDiagramFromSeedSets(gridTiles, plateTileSets, addSubPlatesAfterIteration);

	plateTileSets.AddZeroed(numberOfSubPlateSeeds);
	for (int it = numberOfPlateSeeds; it < numberOfPlateSeeds+numberOfSubPlateSeeds;++it)
	{
		FGridTileSet& tileSet = plateTileSets[it];
		tileSet.setIndex = it;
		int32 seedTile;
		do
		{
			seedTile = randStream.RandRange(0, gridOwner->numTiles - 1);
		} while (!addTileToTileSet(tileSet, seedTile, gridTiles));
		tileSet.setTags.Add(uint8(EPlateTypeEnum::PT_Unassigned));
	}
	createVoronoiDiagramFromSeedSets(gridTiles, plateTileSets);

	// rebuild the plates from a random set of seed tiles inside of the plate
	gridTiles = gridGen->getTiles();
	for (FGridTileSet& tileSet : plateTileSets)
	{
		int32 numSubSeeds = tileSet.containedTiles.Num() / 30 + 1;
		TArray<int32> subSeedNums;
		for (int32 subseedNum = 0; subseedNum < numSubSeeds; ++subseedNum)
		{
			int32 newSubSeed = randStream.RandRange(0, tileSet.containedTiles.Num()-1);
			subSeedNums.Add(tileSet.containedTiles[newSubSeed]);
		}
		tileSet.boarderEdges.Empty();
		tileSet.containedTiles.Empty();
		for (const int32& subSeed:subSeedNums)
		{
			addTileToTileSet(tileSet, subSeed, gridTiles);
		}
	}

	createVoronoiDiagramFromSeedSets(gridTiles, plateTileSets);

	//finally we're going to sort by plate size
	plateTileSets.Sort([](const FGridTileSet& set1, const FGridTileSet& set2)->bool
	{
		return set1.containedTiles.Num() > set2.containedTiles.Num();
	});

	gridOwner->GridTileSets.Add(ETileSetTypeEnum::ST_PLATE, plateTileSets);
}

void UContinentGenerator::calculateLandMasses()
{
	//first we need to know which plates are primarily land and which are primarily water
	TArray<FGridTileSet>& plateTileSets = gridOwner->GridTileSets[ETileSetTypeEnum::ST_PLATE];
	float effectivePercentOcean = 0.0;
	int32 nextSetIndex = 0;
	float smallestRemainingSet = 1.0;
	int32 numSetsRemaining = plateTileSets.Num();
	while (numSetsRemaining != 0)
	{
		if (plateTileSets[nextSetIndex].setTags[0] != uint8(EPlateTypeEnum::PT_Unassigned))
		{
			nextSetIndex++;
		}

		float tileSetCoverage = plateTileSets[nextSetIndex].containedTiles.Num()*1.0 / gridOwner->numTiles;
		if (tileSetCoverage < percentOcean - effectivePercentOcean)
		{
			effectivePercentOcean += tileSetCoverage;
			plateTileSets[nextSetIndex].setTags[0] = uint8(EPlateTypeEnum::PT_Ocean);
			++nextSetIndex;
			--numSetsRemaining;
		}
		else if (tileSetCoverage < smallestRemainingSet)
		{
			smallestRemainingSet = tileSetCoverage;
		}
		else if (tileSetCoverage > percentOcean - effectivePercentOcean)
		{
			plateTileSets[nextSetIndex].setTags[0] = uint8(EPlateTypeEnum::PT_Land);
			--numSetsRemaining;
		}

		++nextSetIndex;
		if (nextSetIndex >= plateTileSets.Num())
		{
			nextSetIndex = 0;
		}
	}

	//categorize plate boundaries
	FMath::RandInit(landMassingSeed);


	FGridTileSet landTileSet;
	FGridTileSet oceanTileSet;
	for (FGridTileSet& plateSet : plateTileSets)
	{
		if (plateSet.setTags[0] == uint8(EPlateTypeEnum::PT_Ocean))
		{
			TArray<FGridTileSet> subdividedPlate = subdividePlate(plateSet, FMath::Rand());
			addTileSetToTileSet(oceanTileSet, subdividedPlate[0]);
			addTileSetToTileSet(landTileSet, subdividedPlate[1]);
		}
		else
		{
			TArray<FGridTileSet> subdividedPlate = subdividePlate(plateSet, FMath::Rand());
			addTileSetToTileSet(landTileSet, subdividedPlate[0]);
			addTileSetToTileSet(oceanTileSet, subdividedPlate[1]);
		}
	}


	gridOwner->GridTileSets.Add(ETileSetTypeEnum::ST_TERRAIN_GROUP);
	gridOwner->GridTileSets[ETileSetTypeEnum::ST_TERRAIN_GROUP].Add(landTileSet);
	gridOwner->GridTileSets[ETileSetTypeEnum::ST_TERRAIN_GROUP].Add(oceanTileSet);
}

void UContinentGenerator::createVoronoiDiagramFromSeedSets(GridTilePtrList &gridTiles, TArray<FGridTileSet>& seedSets,
	const uint32& maxNumIterations /*= -1*/)
{
	//produce our voronoi diagram using Manhattan distances
	int32 currentIt = 0;
	while (gridTiles.Num() != 0)
	{
		for (FGridTileSet& tileSet : seedSets)
		{
			TArray<int32> startBoarderEdges = tileSet.boarderEdges;
			for (const uint32& edgeNum : startBoarderEdges)
			{
				GridTilePtrList edgeTiles = gridGen->getEdge(edgeNum)->getTiles();
				addTileToTileSet(tileSet, edgeTiles[0]->getIndex(), gridTiles);
				addTileToTileSet(tileSet, edgeTiles[1]->getIndex(), gridTiles);
			}
		}
		if (maxNumIterations == currentIt)
		{
			break;
		}
		currentIt++;
	}
}

TArray<FGridTileSet> UContinentGenerator::subdivideSetIntoSubgroups(const FGridTileSet& tileSet, const TArray<int32> subgroupSeedCount, const int32& groupSeed)
{
	FRandomStream randStream(groupSeed);
	TArray<FGridTileSet> subTileSets;
	subTileSets.SetNumZeroed(subgroupSeedCount.Num());
	GridTilePtrList gridTiles = gridGen->getTiles();
	gridTiles.RemoveAll([&tileSet](const GridTilePtr& testTile)->bool
	{
		return !tileSet.containedTiles.Contains(testTile->getIndex());
	});

	for (int32 subSetNum = 0; subSetNum < subgroupSeedCount.Num();++subSetNum)
	{
		FGridTileSet& subTileSet = subTileSets[subSetNum];
		int32 numSubSeeds = subgroupSeedCount[subSetNum];
		TArray<int32> subSeedNums;
		for (int32 subseedNum = 0; subseedNum < numSubSeeds;)
		{
			int32 newSubSeed = randStream.RandRange(0, tileSet.containedTiles.Num() - 1);
			newSubSeed = tileSet.containedTiles[newSubSeed];
			if (addTileToTileSet(subTileSet, newSubSeed, gridTiles))
			{
				++subseedNum;
			}
		}
	}

	createVoronoiDiagramFromSeedSets(gridTiles, subTileSets);
	return subTileSets;
}

bool UContinentGenerator::addTileToTileSet(FGridTileSet& tileSet, const uint32& seedTile, GridTilePtrList& availableTiles)
{
	if (!tileSet.containedTiles.Contains(seedTile) && availableTiles.Contains(gridGen->getTile(seedTile)))
	{
		GridTilePtr newTile = gridGen->getTile(seedTile);
		availableTiles.Remove(newTile);
		tileSet.containedTiles.Add(seedTile);
		GridEdgePtrList tileEdges = newTile->getEdges();
		for (const GridEdgePtr& edgePtr : tileEdges)
		{
			if (!tileSet.boarderEdges.Contains(edgePtr->getIndex()))
			{
				tileSet.boarderEdges.Add(edgePtr->getIndex());
			}
			else
			{
				tileSet.boarderEdges.Remove(edgePtr->getIndex());
			}
		}
		return true;
	}
	return false;
}

TArray<int32> UContinentGenerator::getSetBorderTiles(const FGridTileSet& tileSet) const
{
	TArray<int32> borderTiles;
	for (const int32& edgeIndex:tileSet.boarderEdges)
	{
		GridTilePtrList edgeTiles = gridGen->getEdge(edgeIndex)->getTiles();
		if (tileSet.containedTiles.Contains(edgeTiles[0]->getIndex()))
		{
			borderTiles.AddUnique(edgeTiles[0]->getIndex());
		}
		else
		{
			borderTiles.AddUnique(edgeTiles[1]->getIndex());
		}
	}
	return borderTiles;
}

void UContinentGenerator::addTileSetToTileSet(FGridTileSet& set1, const FGridTileSet& set2)
{
	GridTilePtrList gridTiles = gridGen->getTiles();
	for (const int32& tileNum : set2.containedTiles)
	{
		addTileToTileSet(set1, tileNum, gridTiles);
	}
}

TArray<FGridTileSet> UContinentGenerator::subdividePlate(const FGridTileSet &plateSet, const int32& plateSeed)
{
	int32 numPrimarySeeds = int32(plateSet.containedTiles.Num() * 0.02) * primaryPlateTypeDensity + minPrimaryPlateTypeSeeds;
	int32 numSecondarySeeds = int32(plateSet.containedTiles.Num() * 0.0005) * secondaryPlateTypeDensity + minSecondaryPlateTypeSeeds;
	TArray<int32> seedNums;
	seedNums.Add(numPrimarySeeds);
	seedNums.Add(numSecondarySeeds);
	return subdivideSetIntoSubgroups(plateSet, seedNums, plateSeed);
}

