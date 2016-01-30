// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrivateHexPlanetDeclarations.h"
/**
 * 
 */
class GridGenerator;

class GridTile
{
public:
	GridTile(const int32& index, const int32& numEdges, GridGenerator* myParent);
	~GridTile();
	GridTilePtrList getNeighbors() const;
	GridEdgePtrList getEdges() const;
	GridNodePtrList getNodes() const;
	const FVector& getPosition() const;
	FVector getPosition(const float& radius) const;
	float getArea(const float& radius) const;
	float getEnclosedVolume(const float& radius) const;
	const int32& getIndex() const;
private:
	friend class GridGenerator;
	GridGenerator* gridOwner;
	const int32 myIndex;
	int32 myEdges[6];
	const int32 myNumEdges;
};
