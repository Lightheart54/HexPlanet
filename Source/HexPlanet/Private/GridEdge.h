// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrivateHexPlanetDeclarations.h"
/**
 * 
 */
class GridEdge
{
public:
	GridEdge(const GridNodePtr& startPoint, const GridNodePtr& endPoint);
	~GridEdge();

	GridTilePtrList getTiles() const;
	GridNodePtrList getEndPoints() const;
	FVector getPosition() const;
	float getLength() const;
	FString mapKey()const;
protected:
	friend class GridGenerator;

	FVector myPosition;
	GridTileWPtrList tiles;
	GridNodeWPtrList nodes;
};
