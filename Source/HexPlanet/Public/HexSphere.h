// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HexSphere.generated.h"

class UProceduralMeshComponent;
class GridGenerator;

UCLASS()
class HEXPLANET_API AHexSphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHexSphere();

	UPROPERTY(VisibleAnywhere)
	float surfaceArea;

	UPROPERTY(VisibleAnywhere)
	float volume;

	UPROPERTY(VisibleAnywhere)
	uint32 numTiles;

	UPROPERTY(EditAnywhere(ClampMin = "0.1", UIMin = "0.1"))
	float radius;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0"))
	uint32 numSubvisions;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void Destroyed() override;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif


protected:
	void rebuildMesh();
	UProceduralMeshComponent* createMeshForTile(const FVector& tileCenter, const FVector& tileCorner1, const uint8& numCorners);
	USceneComponent* gridRoot;
	GridGenerator* gridGenerator;
};
