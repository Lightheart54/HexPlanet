// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridTileComponent.h"
#include "HexSphere.h"


// Sets default values for this component's properties
UGridTileComponent::UGridTileComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	tileKey = "";
	instanceMeshNum = 0;
	mapMesh = nullptr;
	// ...
}


// Called when the game starts
void UGridTileComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGridTileComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

TArray<UGridTileComponent*> UGridTileComponent::getNeighbors() const
{
	TArray<UGridTileComponent*> neighbors;
	if (gridOwner != nullptr)
	{
		for(const auto& neighborKey:gridNeighborKeys)
		{
			neighbors.Add(gridOwner->GetGridTile(neighborKey));
		}
	}
	return neighbors;
}

