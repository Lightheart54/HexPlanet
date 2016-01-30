// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrivateHexPlanetDeclarations.h"

class GridGenerator;

class GridEdge
{
public:
	GridEdge(const int32& index, GridGenerator* myParent);
	~GridEdge();
	 
	void setStartAndEndPoints(const int32& newStartPoint, const int32& newEndPoint);
	GridTilePtrList getTiles() const;
	GridNodePtr getStartPoint() const;
	GridNodePtr getEndPoint() const;
	const FVector& getPosition() const;
	FVector getPosition(const float& radius) const;
	float getLength(const float& radius) const;
	const int32& getIndex() const;
protected:
	friend class GridGenerator;
	GridGenerator* gridOwner;
	const int32 myIndex;
	int32 startPoint;
	int32 endPoint;
	int32 tiles[2];
};
