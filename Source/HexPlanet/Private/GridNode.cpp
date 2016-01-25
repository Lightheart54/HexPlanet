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

GridEdgePtrList GridNode::getEdges() const
{
	return lockList(edges);
}

GridTilePtrList GridNode::getTiles() const
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

GridNodePtrList GridNode::getNodes() const
{
	GridNodePtrList connected;
	for (const GridEdgeWPtr& edge : edges)
	{
		GridEdgePtr edgeptr = edge.Pin();
		GridNodePtrList edgeCorners = edgeptr->getEndPoints();
		if (edgeCorners[0].Get() == this)
		{
			connected.Add(edgeCorners.Last());
		}
		else
		{
			connected.Add(edgeCorners[0]);
		}
	}
	return connected;
}

FVector GridNode::getPosition() const
{
	return myPosition;
}
