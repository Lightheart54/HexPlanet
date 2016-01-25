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
	for (const GridEdgeWPtr& edge : edges)
	{
		GridTilePtrList edgeTiles = edge.Pin()->getTiles();
		for (const GridTilePtr& edgeTile : edgeTiles)
		{
			neighbors.AddUnique(edgeTile);
		}
	}
	return neighbors;
}

GridEdgePtrList GridTile::getEdges() const
{
	return lockList(edges);
}

GridNodePtrList GridTile::getNodes() const
{
	GridNodePtrList myNodes;
	for (const GridEdgeWPtr& edge : edges)
	{
		GridNodePtrList edgeNodes = edge.Pin()->getEndPoints();
		for (const GridNodePtr& edgeNode : edgeNodes)
		{
			myNodes.AddUnique(edgeNode);
		}
	}
	return myNodes;
}

FVector GridTile::getPosition() const
{
	return myPosition;
}

float GridTile::getArea() const
{
	GridEdgePtrList myEdges = getEdges();
	float area = 0.0;
	for (const GridEdgePtr& edge : myEdges)
	{
		GridNodePtrList edgeNodes = edge->getEndPoints();
		float myRadius = FMath::Sqrt(FVector::DotProduct(myPosition, myPosition));
		FVector vec1 = edgeNodes[0]->getPosition() - myPosition;
		float length1 = FMath::Sqrt(FVector::DotProduct(vec1, vec1));
		FVector vec2 = edgeNodes[1]->getPosition() - myPosition;
		float length2 = FMath::Sqrt(FVector::DotProduct(vec2, vec2));
		FVector v1xv2 = FVector::CrossProduct(vec1, vec2);
		area += 0.5*FMath::Sqrt(FVector::DotProduct(v1xv2, v1xv2));
	}
	return area;
}

float GridTile::getEnclosedVolume() const
{
	GridEdgePtrList myEdges = getEdges();
	float volume = 0.0;
	for (const GridEdgePtr& edge : myEdges)
	{
		GridNodePtrList edgeNodes = edge->getEndPoints();
		FVector vec1 = edgeNodes[0]->getPosition() - myPosition;
		FVector vec2 = edgeNodes[1]->getPosition() - myPosition;
		FVector v1xv2 = FVector::CrossProduct(vec1, vec2);
		volume += FMath::Abs(FVector::DotProduct(v1xv2, myPosition)/6);
	}
	return volume;
}
