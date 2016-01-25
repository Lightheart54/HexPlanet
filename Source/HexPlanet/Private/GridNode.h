// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrivateHexPlanetDeclarations.h"

class GridNode
{
public:	
	GridNode(const FVector& _position);
	~GridNode();

	GridEdgePtrList getEdges() const;
	GridTilePtrList getTiles() const;
	GridNodePtrList getNodes() const;
	FVector getPosition() const;

protected:
	friend class GridGenerator;

	GridEdgeWPtrList edges;
	FVector myPosition;
	
};
