// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridEdge.h"
#include "GridNode.h"
#include "GridTile.h"
#include "GridGenerator.h"
#include <limits>


GridEdge::GridEdge(const int32& index, GridGenerator* myParent)
	: myIndex(index), gridOwner(myParent)
{
	tiles[0] = std::numeric_limits<int32>::max();
	tiles[1] = std::numeric_limits<int32>::max();
	startPoint = std::numeric_limits<int32>::max();
	endPoint = std::numeric_limits<int32>::max();
}

GridEdge::~GridEdge()
{
}

void GridEdge::setStartAndEndPoints(const int32& newStartPoint, const int32& newEndPoint)
{
	tiles[0] = startPoint;
	tiles[1] = endPoint;
	startPoint = newStartPoint;
	endPoint = newEndPoint;
	GridNodePtr startNode = gridOwner->getNode(startPoint);
	GridNodePtr endNode = gridOwner->getNode(endPoint);
	bool doneStart = false;
	bool doneEnd = false;
	for (int32 index = 0; index < 3 ; ++index)
	{
		if (!doneStart && startNode->myEdges[index] == std::numeric_limits<int32>::max())
		{
			doneStart = true;
			startNode->myEdges[index] = myIndex;
		}
		if (!doneEnd && endNode->myEdges[index] == std::numeric_limits<int32>::max())
		{
			doneEnd = true;
			endNode->myEdges[index] = myIndex;
		}
	}
}

GridTilePtrList GridEdge::getTiles() const
{
	GridTilePtrList neighbors;
	neighbors.Add(gridOwner->getTile(tiles[0]));
	neighbors.Add(gridOwner->getTile(tiles[1]));
	return neighbors;
}

GridNodePtr GridEdge::getStartPoint() const
{
	return gridOwner->getNode(startPoint);
}

GridNodePtr GridEdge::getEndPoint() const
{
	return gridOwner->getNode(endPoint);
}

const FVector& GridEdge::getPosition() const
{
	return gridOwner->getEdgeLocation(myIndex);
}

FVector GridEdge::getPosition(const float& radius) const
{
	return gridOwner->getEdgeLocation(myIndex, radius);
}

float GridEdge::getLength(const float& radius) const
{
	return radius * FVector::Dist(gridOwner->getNodeLocation(startPoint),
								gridOwner->getNodeLocation(endPoint));
}

const int32& GridEdge::getIndex() const
{
	return myIndex;
}

