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

	inline const GridTilePtrList& getTiles() const
	{
		return tiles;
	}
	inline const GridNodePtr& getStartPoint() const
	{
		return pStartPoint;
	}
	inline const GridNodePtr& getEndPoint() const
	{
		return pEndPoint;
	}
	inline const FVector& getPosition() const
	{
		return myPosition;
	}
	float getLength() const;
	FString mapKey()const;
protected:
	friend class GridGenerator;

	FVector myPosition;
	GridTilePtrList tiles;
	GridNodePtr pStartPoint;
	GridNodePtr pEndPoint;
};
