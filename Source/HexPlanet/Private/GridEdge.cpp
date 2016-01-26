// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridEdge.h"
#include "GridNode.h"
#include "GridTile.h"


GridEdge::GridEdge(const GridNodePtr& startPoint, const GridNodePtr& endPoint)
	: pStartPoint(startPoint), pEndPoint(endPoint)
{
	myPosition = (pStartPoint->getPosition() + pEndPoint->getPosition()) / 2;
}

GridEdge::~GridEdge()
{
}

float GridEdge::getLength() const
{
	return FVector::Dist(pStartPoint->getPosition(), pEndPoint->getPosition());
}

FString GridEdge::mapKey() const
{
	return createKeyForVector(myPosition);
}
