// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridTile.h"
#include "GridNode.h"
#include "GridEdge.h"
#include "GridGenerator.h"
#include <limits>

GridTile::GridTile(const int32& index, const int32& numEdges, GridGenerator* myParent)
	: gridOwner(myParent), myIndex(index), myNumEdges(numEdges)
{
	for (size_t i = 0; i < 6; i++)
	{
		myEdges[i];
	}
}

GridTile::~GridTile()
{
}

GridTilePtrList GridTile::getNeighbors() const
{
	GridEdgePtrList edges = getEdges();
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

GridEdgePtrList GridTile::getEdges() const
{
	GridEdgePtrList edges;
	for (size_t i = 0; i < myNumEdges; i++)
	{
		if (i != 6 || myEdges[i] != std::numeric_limits<int32>::max())
		{
			edges.Add(gridOwner->getEdge(myEdges[i]));
		}
	}
	return edges;
}

GridNodePtrList GridTile::getNodes() const
{
	GridEdgePtrList edges = getEdges();
	GridNodePtrList myNodes;
	for (const GridEdgePtr& edge : edges)
	{
		myNodes.AddUnique(edge->getStartPoint());
		myNodes.AddUnique(edge->getEndPoint());
	}
	return myNodes;
}

const FVector& GridTile::getPosition() const
{
	return gridOwner->getNodeLocation(myIndex);
}

FVector GridTile::getPosition(const float& radius) const
{
	FVector myPosition = FVector::ZeroVector;
	GridEdgePtrList edges = getEdges();
	for (const GridEdgePtr& edge : edges)
	{
		myPosition += edge->getPosition(radius);
	}
	return myPosition/edges.Num();
}

float GridTile::getArea(const float& radius) const
{
	GridEdgePtrList edges = getEdges();
	float area = 0.0;
	FVector myPosition = getPosition(radius);
	for (const GridEdgePtr& edge : edges)
	{
		float myRadius = FMath::Sqrt(FVector::DotProduct(myPosition, myPosition));
		FVector vec1 = edge->getStartPoint()->getPosition(radius) - myPosition;
		float length1 = FMath::Sqrt(FVector::DotProduct(vec1, vec1));
		FVector vec2 = edge->getEndPoint()->getPosition(radius) - myPosition;
		float length2 = FMath::Sqrt(FVector::DotProduct(vec2, vec2));
		FVector v1xv2 = FVector::CrossProduct(vec1, vec2);
		area += 0.5*FMath::Sqrt(FVector::DotProduct(v1xv2, v1xv2));
	}
	return area;
}

float GridTile::getEnclosedVolume(const float& radius) const
{
	float volume = 0.0;
	FVector myPosition = getPosition(radius);
	GridEdgePtrList edges = getEdges();
	for (const GridEdgePtr& edge : edges)
	{
		FVector vec1 = edge->getStartPoint()->getPosition(radius) - myPosition;
		FVector vec2 = edge->getEndPoint()->getPosition(radius) - myPosition;
		FVector v1xv2 = FVector::CrossProduct(vec1, vec2);
		volume += FMath::Abs(FVector::DotProduct(v1xv2, myPosition)/6);
	}
	return volume;
}

const int32& GridTile::getIndex() const
{
	return myIndex;
}
