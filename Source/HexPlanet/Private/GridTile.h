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
	inline const GridEdgePtrList& getEdges() const
	{
		return edges;
	}
	GridNodePtrList getNodes() const;
	inline const FVector& getPosition() const
	{
		return myPosition;
	}
	float getArea() const;
	float getEnclosedVolume() const;
	FString mapKey()const;
private:
	friend class GridGenerator;

	FVector myPosition;
	GridEdgePtrList edges;
};
