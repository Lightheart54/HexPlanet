// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "ContinentGenerator.h"
#include <cassert>


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
	tectonicPlateBoundarySeed = FMath::Rand();

	//land massing settings
	landMassingSeed = FMath::Rand();
	percentOcean = .6;
	minPrimaryPlateTypeSeeds = 1;
	primaryPlateTypeDensity = 3;
	primaryRateAlongFault = .5;
	minSecondaryPlateTypeSeeds = 4;
	secondaryPlateTypeDensity = 0;
	secondRateAlongFault = .5;
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
	if (!gridOwner->GridTileSets.Contains(ETileSetTypeEnum::ST_PLATE))
	{
		gridOwner->GridTileSets.Add(ETileSetTypeEnum::ST_PLATE);
	}
	TArray<FGridTileSet>& plateTileSets = gridOwner->GridTileSets[ETileSetTypeEnum::ST_PLATE];
	FRandomStream randStream(tectonicPlateSeed);
	plateTileSets.SetNumZeroed(numberOfPlateSeeds);
	GridTilePtrList gridTiles = gridGen->getTiles();
	TArray<bool> usedTiles;
	usedTiles.Init(true, gridTiles.Num());
	for (int it = 0; it < numberOfPlateSeeds; ++it)
	{
		FGridTileSet& tileSet = plateTileSets[it];
		tileSet.setIndex = it;
		int32 seedTile;
		do 
		{
			seedTile = randStream.RandRange(0, gridOwner->numTiles - 1);
		} while (!addTileToTileSet(tileSet, seedTile, usedTiles));
		tileSet.setTags.Add(uint8(EPlateTypeEnum::PT_Unassigned));
	}

	createVoronoiDiagramFromSeedSets(usedTiles, plateTileSets, addSubPlatesAfterIteration);

	plateTileSets.AddZeroed(numberOfSubPlateSeeds);
	for (int it = numberOfPlateSeeds; it < numberOfPlateSeeds+numberOfSubPlateSeeds;++it)
	{
		FGridTileSet& tileSet = plateTileSets[it];
		tileSet.setIndex = it;
		int32 seedTile;
		do
		{
			seedTile = randStream.RandRange(0, gridOwner->numTiles - 1);
		} while (!addTileToTileSet(tileSet, seedTile, usedTiles));
		tileSet.setTags.Add(uint8(EPlateTypeEnum::PT_Unassigned));
	}
	createVoronoiDiagramFromSeedSets(usedTiles, plateTileSets);

	// rebuild the plates from a random set of seed tiles inside of the plate
	gridTiles = gridGen->getTiles(); 
	usedTiles.Init(true, gridTiles.Num());
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
			addTileToTileSet(tileSet, subSeed, usedTiles);
		}
	}

	createVoronoiDiagramFromSeedSets(usedTiles, plateTileSets);

	//finally we're going to sort by plate size
	plateTileSets.Sort([](const FGridTileSet& set1, const FGridTileSet& set2)->bool
	{
		return set1.containedTiles.Num() > set2.containedTiles.Num();
	});
}

void UContinentGenerator::setUpPlateBoundaries()
{
	TArray<FGridTileSet>& plateTileSets = gridOwner->GridTileSets[ETileSetTypeEnum::ST_PLATE];
	if (!gridOwner->GridTileSets.Contains(ETileSetTypeEnum::ST_PLATE_BOUNDARY))
	{
		gridOwner->GridTileSets.Add(ETileSetTypeEnum::ST_PLATE_BOUNDARY);
	}
	TArray<FGridTileSet>& plateBoundarySet = gridOwner->GridTileSets[ETileSetTypeEnum::ST_PLATE_BOUNDARY];

	GridTilePtrList availableTiles = gridGen->getTiles();
	TArray<bool> tileAvailability;
	tileAvailability.Init(true, availableTiles.Num());

	TArray<bool> edgeAvailability;
	edgeAvailability.Init(true,gridGen->getEdges().Num());
	FMath::RandInit(tectonicPlateBoundarySeed);
	int8 maxBoundaryTypeValue = int8(EPlateBoarderType::PB_CONVERGENT);
	for (FGridTileSet& plateSet : plateTileSets)
	{
		int32 plate1Index = plateSet.setIndex;
		int32 plate2Index = -1;
		int32 currentBoundaryIndex = -1;
		for (const int32& edgeIndex : plateSet.boarderEdges)
		{
			if (!edgeAvailability[edgeIndex])
			{
				continue;
			}
			GridEdgePtr nextEdge = gridGen->getEdge(edgeIndex);
			edgeAvailability[edgeIndex] = false;

			FGridTile& tile1 = gridOwner->GridTiles[nextEdge->getTileIndexes()[0]];
			FGridTile& tile2 = gridOwner->GridTiles[nextEdge->getTileIndexes()[1]];
			int32 nextPlate2Index;
			if (tile1.owningTileSets[int8(ETileSetTypeEnum::ST_PLATE)] == plate1Index)
			{
				nextPlate2Index = tile2.owningTileSets[int8(ETileSetTypeEnum::ST_PLATE)];
			}
			else
			{
				nextPlate2Index = tile1.owningTileSets[int8(ETileSetTypeEnum::ST_PLATE)];
			}

			if (nextPlate2Index != plate2Index)
			{
				plate2Index = nextPlate2Index;
				//see if we already have a boundary for this pairing
				FGridTileSet* nextPlateSet = plateBoundarySet.FindByPredicate([&plate1Index, &plate2Index](const FGridTileSet& boundarySet)->bool
				{
					return (boundarySet.setTags[1] == plate1Index && boundarySet.setTags[2] == plate2Index)
						|| (boundarySet.setTags[2] == plate1Index && boundarySet.setTags[1] == plate2Index);
				});
				if (nextPlateSet != nullptr)
				{
					currentBoundaryIndex = nextPlateSet->setIndex;
				}
				else
				{
					//otherwise make a new one
					FGridTileSet newBoundary;
					newBoundary.setType = ETileSetTypeEnum::ST_PLATE_BOUNDARY;
					newBoundary.setTags.Add(FMath::RandRange(0, maxBoundaryTypeValue));
					newBoundary.setTags.Add(plate1Index);
					newBoundary.setTags.Add(plate2Index);
					newBoundary.setIndex = plateBoundarySet.Num();
					currentBoundaryIndex = plateBoundarySet.Add(newBoundary);
				}
			}
			
			if (tile1.owningTileSets.Num() < int8(ETileSetTypeEnum::ST_PLATE_BOUNDARY) + 1)
			{
				tile1.owningTileSets.Add(currentBoundaryIndex);
				addTileToTileSet(plateBoundarySet[currentBoundaryIndex], tile1.tileIndex, tileAvailability);
			}
			if (tile2.owningTileSets.Num() < int8(ETileSetTypeEnum::ST_PLATE_BOUNDARY) + 1)
			{
				tile2.owningTileSets.Add(currentBoundaryIndex);
				addTileToTileSet(plateBoundarySet[currentBoundaryIndex], tile2.tileIndex, tileAvailability);
			}
		}
	}
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
		if (nextSetIndex >= plateTileSets.Num())
		{
			nextSetIndex = 0;
		}

		if (plateTileSets[nextSetIndex].setTags[0] != uint8(EPlateTypeEnum::PT_Unassigned))
		{
			nextSetIndex++;
			continue;
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
	}

	//categorize plate boundaries
	FGridTileSet landTileSet;
	landTileSet.setIndex = 0;
	landTileSet.setType = ETileSetTypeEnum::ST_TERRAIN_GROUP;
	landTileSet.setTags.Add(int8(EPlateTypeEnum::PT_Land));
	FGridTileSet oceanTileSet;
	oceanTileSet.setIndex = 1;
	oceanTileSet.setType = ETileSetTypeEnum::ST_TERRAIN_GROUP;
	oceanTileSet.setTags.Add(int8(EPlateTypeEnum::PT_Ocean));
	gridOwner->GridTileSets.Add(ETileSetTypeEnum::ST_TERRAIN_GROUP);
	gridOwner->GridTileSets[ETileSetTypeEnum::ST_TERRAIN_GROUP].Add(landTileSet);
	gridOwner->GridTileSets[ETileSetTypeEnum::ST_TERRAIN_GROUP].Add(oceanTileSet);

	FMath::RandInit(landMassingSeed);
	for (FGridTileSet& plateSet : plateTileSets)
	{
		subdividePlate(plateSet);
	}

}

void UContinentGenerator::createVoronoiDiagramFromSeedSets(TArray<bool>& availableTiles, TArray<FGridTileSet>& seedSets,
	const uint32& maxNumIterations /*= -1*/)
{
	//produce our voronoi diagram using Manhattan distances
	int32 currentIt = 0;
	bool tryAgain = true;
	while (tryAgain)
	{
		tryAgain = false;
		for (FGridTileSet& tileSet : seedSets)
		{
			TArray<int32> startBoarderEdges = tileSet.boarderEdges;
			for (const uint32& edgeNum : startBoarderEdges)
			{
				GridTilePtrList edgeTiles = gridGen->getEdge(edgeNum)->getTiles();
				tryAgain |= addTileToTileSet(tileSet, edgeTiles[0]->getIndex(), availableTiles);
				tryAgain |= addTileToTileSet(tileSet, edgeTiles[1]->getIndex(), availableTiles);
			}
		}
		if (maxNumIterations == currentIt)
		{
			break;
		}
		currentIt++;
	}
}

bool UContinentGenerator::addTileToTileSet(FGridTileSet& tileSet, const uint32& seedTile, TArray<bool>& availableTiles)
{
	if (!tileSet.containedTiles.Contains(seedTile) && availableTiles[seedTile])
	{
		GridTilePtr newTile = gridGen->getTile(seedTile);
		availableTiles[seedTile] = false;
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
		FGridTile& gridTile = gridOwner->GridTiles[seedTile];
		gridTile.owningTileSets.SetNumZeroed(int8(tileSet.setType)+1);
		gridTile.owningTileSets[int8(tileSet.setType)] = tileSet.setIndex;
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
	TArray<bool> usedTiles;
	usedTiles.Init(true, gridTiles.Num());
	for (const int32& tileNum : set2.containedTiles)
	{
		addTileToTileSet(set1, tileNum, usedTiles);
	}
}

void UContinentGenerator::subdividePlate(const FGridTileSet &plateSet)
{
	int32 numPrimarySeeds = int32(plateSet.containedTiles.Num() * 0.02) * primaryPlateTypeDensity + minPrimaryPlateTypeSeeds;
	int32 numSecondarySeeds = int32(plateSet.containedTiles.Num() * 0.0005) * secondaryPlateTypeDensity + minSecondaryPlateTypeSeeds;
	int32 numLandSeeds;
	int32 numOceanSeeds;
	if (plateSet.setTags[0] == int8(EPlateTypeEnum::PT_Land))
	{
		numLandSeeds = numPrimarySeeds;
		numOceanSeeds = numSecondarySeeds;
	}
	else
	{
		numOceanSeeds = numPrimarySeeds;
		numLandSeeds = numSecondarySeeds;
	}

	FGridTileSet landTileSet;
	landTileSet.setIndex = 0;
	landTileSet.setType = ETileSetTypeEnum::ST_TERRAIN_GROUP;
	landTileSet.setTags.Add(int8(EPlateTypeEnum::PT_Land));
	FGridTileSet oceanTileSet;
	oceanTileSet.setIndex = 1;
	oceanTileSet.setType = ETileSetTypeEnum::ST_TERRAIN_GROUP;
	oceanTileSet.setTags.Add(int8(EPlateTypeEnum::PT_Ocean));

	GridTilePtrList gridTiles = gridGen->getTiles();
	TArray<bool> tileAvailability;
	tileAvailability.Init(false, gridTiles.Num());
	for (const int32& plateId : plateSet.containedTiles)
	{
		tileAvailability[plateId] = true;
	}

	//now we need to pre-seed the plate boundaries
	TArray<int32> plateBorderTiles = getSetBorderTiles(plateSet);
	for (const int32& borderTileIndex : plateBorderTiles)
	{
		FGridTile& borderTile = gridOwner->GridTiles[borderTileIndex];
		int32 borderIndex = borderTile.owningTileSets[int8(ETileSetTypeEnum::ST_PLATE_BOUNDARY)];
		FGridTileSet& borderSet = gridOwner->GridTileSets[ETileSetTypeEnum::ST_PLATE_BOUNDARY][borderIndex];
		assert(borderSet.containedTiles.Contains(borderTile.tileIndex));
		float chance = FMath::FRandRange(0.0, 1.0);
		if (borderSet.setTags[0] == int8(EPlateBoarderType::PB_CONVERGENT))
		{
			if (plateSet.setTags[0] == int8(EPlateTypeEnum::PT_Land)
				&& chance <= primaryRateAlongFault)
			{
				addTileToTileSet(landTileSet, borderTile.tileIndex, tileAvailability);
			}
			else if (chance <= secondRateAlongFault)
			{
				addTileToTileSet(landTileSet, borderTile.tileIndex, tileAvailability);
			}
		}
		else
		{
			if (plateSet.setTags[0] == int8(EPlateTypeEnum::PT_Ocean)
				&& chance <= primaryRateAlongFault)
			{
				addTileToTileSet(oceanTileSet, borderTile.tileIndex, tileAvailability);
			}
			else if (chance <= secondRateAlongFault)
			{
				addTileToTileSet(oceanTileSet, borderTile.tileIndex, tileAvailability);
			}
		}
	}

	//next add the prescribed number of starting seeds
	for (int it = 0; it < numLandSeeds; ++it)
	{
		int32 seedTile;
		do
		{
			seedTile = FMath::RandRange(0, plateSet.containedTiles.Num() - 1);
			seedTile = plateSet.containedTiles[seedTile];
		} while (!addTileToTileSet(landTileSet, seedTile, tileAvailability));
	}
	for (int it = 0; it < numOceanSeeds; ++it)
	{
		int32 seedTile;
		do
		{
			seedTile = FMath::RandRange(0, plateSet.containedTiles.Num() - 1);
			seedTile = plateSet.containedTiles[seedTile];
		} while (!addTileToTileSet(oceanTileSet, seedTile, tileAvailability));
	}

	TArray<FGridTileSet> terrianTypes;
	terrianTypes.Add(landTileSet);
	terrianTypes.Add(oceanTileSet);
	createVoronoiDiagramFromSeedSets(tileAvailability, terrianTypes);

	addTileSetToTileSet(gridOwner->GridTileSets[ETileSetTypeEnum::ST_TERRAIN_GROUP][landTileSet.setIndex], terrianTypes[0]);
	addTileSetToTileSet(gridOwner->GridTileSets[ETileSetTypeEnum::ST_TERRAIN_GROUP][oceanTileSet.setIndex], terrianTypes[1]);
}