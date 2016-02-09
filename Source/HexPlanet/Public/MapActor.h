// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GridMesher.h"
#include "SphereGrid.h"
#include "MapActor.generated.h"

UCLASS()
class HEXPLANET_API AMapActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MapRepresentation")
		UGridMesher* gridMesher;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MapRepresentation")
		USphereGrid* sphereGrid;	
	
};
