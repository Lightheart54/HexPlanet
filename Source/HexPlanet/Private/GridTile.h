// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrivateHexPlanetDeclarations.h"
/**
 * 
 */
class GridTile
{
public:
	GridTile(const GridEdgePtrList& tileEdges);
	~GridTile();
	GridTilePtrList getNeighbors() const;
	GridEdgePtrList getEdges() const;
	GridNodePtrList getNodes() const;
	FVector getPosition() const;
	float getArea() const;
	float getEnclosedVolume() const;

private:
	friend class GridGenerator;

	FVector myPosition;
	GridEdgeWPtrList edges;
};
