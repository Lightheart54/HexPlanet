// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrivateHexPlanetDeclarations.h"

class GridNode
{
public:	
	GridNode(const FVector& _position);
	~GridNode();

	inline const GridEdgePtrList& getEdges() const
	{
		return edges;
	}
	GridTilePtrList getTiles() const;
	GridNodePtrList getNodes() const;
	FString mapKey() const;
	inline const FVector& getPosition() const
	{
		return myPosition;
	}

protected:
	friend class GridGenerator;

	GridEdgePtrList edges;
	FVector myPosition;
	
};