// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridNode.h"
#include "GridEdge.h"
#include "GridTile.h"

GridNode::GridNode(const FVector& _position) : edges()
{
	myPosition = _position;
}

GridNode::~GridNode()
{

}

GridTilePtrList GridNode::getTiles() const
{
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

FString GridNode::mapKey() const
{
	return createKeyForVector(myPosition);
}
