// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "GridGenerator.h"
#include "HexSphere.h"
#include "ContinentGenerator.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UContinentGenerator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UContinentGenerator();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void installGrid(AHexSphere* myOwner, GridGenerator* gridGenerator);

	TArray<FGridTileSet> buildTectonicPlates(const int32& numberOfPlates, const int32& plateSeed);
	void addTileToTileSet(FGridTileSet& tileSet, const uint32& seedTile, GridTilePtrList& availableTiles);
protected:
	AHexSphere* gridOwner;
	GridGenerator* gridGen;
	
};
