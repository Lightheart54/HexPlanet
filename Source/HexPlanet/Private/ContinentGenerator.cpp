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
}

TArray<FGridTileSet> UContinentGenerator::buildTechtonicPlates(const int32& numberOfPlates, const int32& plateSeed)
{
	FRandomStream randStream(plateSeed);
	TArray<FGridTileSet> plateSets;
	plateSets.SetNumZeroed(numberOfPlates);
	GridTilePtrList gridTiles = gridGen->getTiles();	
	for (FGridTileSet& tileSet : plateSets)
	{
		int32 seedTile = randStream.RandRange(0, gridTiles.Num());
		addTileToTileSet(tileSet, seedTile, gridTiles);
	}

	//produce our voronoi diagram using Manhattan distances
	while (gridTiles.Num() != 0)
	{
		for (FGridTileSet& tileSet : plateSets)
		{
			TArray<int32> startBoarderEdges = tileSet.boarderEdges;
			for (const uint32& edgeNum : startBoarderEdges)
			{
				GridTilePtrList edgeTiles = gridGen->getEdge(edgeNum)->getTiles();
				addTileToTileSet(tileSet, edgeTiles[0]->getIndex(), gridTiles);
				addTileToTileSet(tileSet, edgeTiles[1]->getIndex(), gridTiles);
			}
		}
	}

	return plateSets;
}

void UContinentGenerator::addTileToTileSet(FGridTileSet& tileSet, const uint32& seedTile, GridTilePtrList& availableTiles)
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
	}
}

