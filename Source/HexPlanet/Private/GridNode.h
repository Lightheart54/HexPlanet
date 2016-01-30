// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrivateHexPlanetDeclarations.h"
class GridGenerator;

class GridNode
{
public:	
	GridNode(const int32& index, GridGenerator* myParent);
	~GridNode();

	GridEdgePtrList getEdges() const;
	GridTilePtrList getTiles() const;
	GridNodePtrList getNodes() const;
	const int32& getIndex();
	const FVector& getPosition() const;
	FVector getPosition(const float& radius) const;

protected:
	friend class GridGenerator;
	friend class GridEdge;
	GridGenerator* gridOwner;
	const int32 myIndex;
	int32 myEdges[3];
};