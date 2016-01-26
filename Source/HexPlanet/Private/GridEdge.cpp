// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridEdge.h"
#include "GridNode.h"
#include "GridTile.h"


GridEdge::GridEdge(const GridNodePtr& startPoint, const GridNodePtr& endPoint)
{
	nodes.Add(startPoint);
	nodes.Add(endPoint);
	FVector startPos = startPoint->getPosition();
	FVector endPos = endPoint->getPosition();
	myPosition = (startPos + endPos) / 2;
}

GridEdge::~GridEdge()
{
}

GridTilePtrList GridEdge::getTiles() const
{
	return lockList(tiles);
}

GridNodePtrList GridEdge::getEndPoints() const
{
	return lockList(nodes);
}

FVector GridEdge::getPosition() const
{
	return myPosition;
}

float GridEdge::getLength() const
{
	FVector startPoint = nodes[0].Pin()->getPosition();
	FVector endPoint = nodes[1].Pin()->getPosition();
	return FVector::Dist(startPoint, endPoint);
}

FString GridEdge::mapKey() const
{
	return createKeyForVector(myPosition);
}
