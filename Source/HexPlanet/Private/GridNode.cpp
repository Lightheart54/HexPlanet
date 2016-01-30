// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridNode.h"
#include "GridEdge.h"
#include "GridTile.h"
#include "GridGenerator.h"
#include <limits>

GridNode::GridNode(const int32& index, GridGenerator* myParent) : gridOwner(myParent), myIndex(index)
{
	myEdges[0] = std::numeric_limits<int32>::max();
	myEdges[1] = std::numeric_limits<int32>::max();
	myEdges[2] = std::numeric_limits<int32>::max();
}

GridNode::~GridNode()
{

}

GridEdgePtrList GridNode::getEdges() const
{
	GridEdgePtrList edges;
	edges.Add(gridOwner->getEdge(myEdges[0]));
	edges.Add(gridOwner->getEdge(myEdges[1]));
	edges.Add(gridOwner->getEdge(myEdges[2]));
	return edges;
}

GridTilePtrList GridNode::getTiles() const
{
	GridEdgePtrList edges = getEdges();
	GridTilePtrList neighbors;
	for (const GridEdgePtr& edge : edges)
	{
		GridTilePtrList edgeTiles = edge->getTiles();
		for (const GridTilePtr& edgeTile : edgeTiles)
		{
			neighbors.AddUnique(edgeTile);
		}
	}
	return neighbors;
}

GridNodePtrList GridNode::getNodes() const
{
	GridEdgePtrList edges = getEdges();
	GridNodePtrList connected;
	for (const GridEdgePtr& edge : edges)
	{
		if (edge->getStartPoint() == this)
		{
			connected.Add(edge->getEndPoint());
		}
		else
		{
			connected.Add(edge->getStartPoint());
		}
	}
	return connected;
}

const int32& GridNode::getIndex()
{
	return myIndex;
}

const FVector& GridNode::getPosition() const
{
	return gridOwner->getNodeLocation(myIndex);
}

FVector GridNode::getPosition(const float& radius) const
{
	return gridOwner->getNodeLocation(myIndex,radius);
}
