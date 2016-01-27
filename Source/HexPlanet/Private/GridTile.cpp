// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridTile.h"
#include "GridNode.h"
#include "GridEdge.h"

GridTile::GridTile(const GridEdgePtrList& tileEdges)
{
	myPosition = FVector::ZeroVector;
	for (const GridEdgePtr& tileEdge : tileEdges)
	{
		edges.Add(tileEdge);
		myPosition += tileEdge->getPosition();
	}
	myPosition /= tileEdges.Num();
}

GridTile::~GridTile()
{
}

GridTilePtrList GridTile::getNeighbors() const
{
	GridTilePtrList neighbors;
	for (const GridEdgePtr& edge : edges)
	{
		GridTilePtrList edgeTiles = edge->getTiles();
		if (edgeTiles[0] == this)
		{
			neighbors.Add(edgeTiles[1]);
		}
		else
		{
			neighbors.Add(edgeTiles[0]);
		}
	}
	return neighbors;
}

GridNodePtrList GridTile::getNodes() const
{
	GridNodePtrList myNodes;
	for (const GridEdgePtr& edge : edges)
	{
		myNodes.AddUnique(edge->getStartPoint());
		myNodes.AddUnique(edge->getEndPoint());
	}
	return myNodes;
}

float GridTile::getArea() const
{
	float area = 0.0;
	for (const GridEdgePtr& edge : edges)
	{
		float myRadius = FMath::Sqrt(FVector::DotProduct(myPosition, myPosition));
		FVector vec1 = edge->getStartPoint()->getPosition() - myPosition;
		float length1 = FMath::Sqrt(FVector::DotProduct(vec1, vec1));
		FVector vec2 = edge->getEndPoint()->getPosition() - myPosition;
		float length2 = FMath::Sqrt(FVector::DotProduct(vec2, vec2));
		FVector v1xv2 = FVector::CrossProduct(vec1, vec2);
		area += 0.5*FMath::Sqrt(FVector::DotProduct(v1xv2, v1xv2));
	}
	return area;
}

float GridTile::getEnclosedVolume() const
{
	float volume = 0.0;
	for (const GridEdgePtr& edge : edges)
	{
		FVector vec1 = edge->getStartPoint()->getPosition() - myPosition;
		FVector vec2 = edge->getEndPoint()->getPosition() - myPosition;
		FVector v1xv2 = FVector::CrossProduct(vec1, vec2);
		volume += FMath::Abs(FVector::DotProduct(v1xv2, myPosition)/6);
	}
	return volume;
}

FString GridTile::mapKey() const
{
	return createKeyForVector(myPosition);
}
