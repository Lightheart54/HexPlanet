// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrivateHexPlanetDeclarations.h"
#include "GridTile.h"
#include "GridEdge.h"
#include "GridNode.h"
/**
 * 
 */
class GridGenerator
{
public:
	GridGenerator(const float& _radius = 1.0, const uint8& _numSubdivisions = 0);

	~GridGenerator();

	void rebuildGrid(const float& newRadius = 1.0, const uint8& newNumSubdivisions = 0);
	void rebuildGridForAGivenTileArea(const float& newRadius, const float& tileArea);

	GridTilePtr getTile(const FVector& vec) const;
	GridEdgePtr getEdge(const FVector& vec) const;
	GridNodePtr getNode(const FVector& vec) const;

	GridTilePtrList getTiles() const;
	GridEdgePtrList getEdges() const;
	GridNodePtrList getNodes() const;
	float getRadius() const;

	float getVolume() const;
	float getSurfaceArea() const;

private:
	void buildNewGrid();
	void createBaseGrid();
	void subdivideGrid();
	void cleanUpMemberWPtrLists() const;


	GridNodePtr createNode(const FVector& pos);
	GridEdgePtr createEdge(const GridNodePtr& startPoint, const GridNodePtr& endPoint);
	GridEdgePtr createAndRegisterEdge(const GridNodePtr& startPoint, const GridNodePtr& endPoint);
	GridTilePtr createTile(const GridEdgePtrList& edgeLoop);
	void registerTileWithEdges(const GridTilePtr& tileptr);
	GridTilePtr createTile(const TArray<FVector>& nodeLocs);
	GridTilePtr createTile(const GridNodePtrList& loopNodes);
	GridEdgePtrList createEdgeLoop(const TArray<FVector>& nodeLocs);
	GridEdgePtrList createEdgeLoop(GridNodePtrList loopNodes);

	float radius;
	uint8 numSubdivisions;

	GridTileMap tiles;
	GridNodeMap nodes;
	GridEdgeMap edges;
};
