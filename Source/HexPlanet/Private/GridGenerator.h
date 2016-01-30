// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrivateHexPlanetDeclarations.h"
#include "GridTile.h"
#include "GridEdge.h"
#include "GridNode.h"
/**
 * 
 */

 //For UE4 Profiler ~ Stat
//DECLARE_CYCLE_STAT(TEXT("GridGeneration"), STATGROUP_GridGen, STATCAT_Advanced);

class GridGenerator
{
public:
	GridGenerator(const uint8& _numSubdivisions = 0);

	~GridGenerator();

	void rebuildGrid(const uint8& newNumSubdivisions = 0);
	const uint8& getNumSubdivisions() const;

	
	FVector getEdgeLocation(const int32& edgeIndex, const float& radius) const;
	const FVector& getEdgeLocation(const int32& edgeIndex) const;
	FVector getNodeLocation(const int32& nodeIndex, const float& radius) const;
	const FVector& getNodeLocation(const int32& nodeIndex) const;
	FVector getTileLocation(const int32& tileIndex, const float& radius) const;
	const FVector& getTileLocation(const int32& tileIndex) const;

	GridTilePtr getTile(const int32& tileIndex) const;
	GridTilePtrList getTiles() const;
	GridEdgePtr getEdge(const int32& edgeIndex) const;
	GridEdgePtrList getEdges() const;
	GridNodePtr getNode(const int32& nodeIndex) const;
	GridNodePtrList getNodes() const;

	float getVolume(const float& radius) const;
	float getSurfaceArea(const float& radius) const;
	FVector findAveragePoint(const FVector& vec1, const FVector& vec2) const;
	FVector findAveragePoint(const FVector& vec1, const FVector& vec2, const FVector& vec3) const;

private:
	void buildNewGrid();
	void createBaseGrid();
	void subdivideEdge(const int32& baseEdge, TMap<int32, TArray<int32>>& nodeToNodesMapOut);

	void subdivideGrid(bool skipTiles = false);
	void cleanUpMemberWPtrLists() const;


	int32 createNode(const FVector& pos);
	int32 createEdge(const int32& startPoint, const int32& endPoint);
	int32 createTile(const int32& baseNodeNum, TArray<int32> tileNodes);

	uint8 numSubdivisions;
	int32 nextNodeNumber;
	int32 nextTileNumber;
	int32 nextEdgeNumber;
	TArray<FVector> edgeLocations;
	TArray<FVector> nodeLocations;
	GridTilePtrList tiles;
	GridNodePtrList nodes;
	GridEdgePtrList edges;
};
