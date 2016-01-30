// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "GridTileComponent.generated.h"

class AHexSphere;

UCLASS(ClassGroup="HexGrid")
class HEXPLANET_API UGridTileComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGridTileComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
		
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GridNavigation")
	TArray<UGridTileComponent*> getNeighbors() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridNavigation")
	TArray<int32> gridNeighborKeys;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridNavigation")
	int32 tileKey;

	UInstancedStaticMeshComponent* mapMesh;
	int32 instanceMeshNum;
	AHexSphere* gridOwner;
	
};
