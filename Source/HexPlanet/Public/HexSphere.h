// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HexSphere.generated.h"

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

	UPROPERTY(EditAnywhere(ClampMin = "0.1", UIMin = "0.1", ClampMax = "1.1", UIMax = "1.1"))
	float tileFillRatio;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0"))
	uint32 numSubvisions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	UStaticMesh* PentagonMesh; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	float PentagonMeshInnerRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	UStaticMesh* HexagonMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	float HexagonMeshInnerRadius;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void Destroyed() override;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);


	UPROPERTY(EditAnywhere, Category = "Debug Grid Display")
		bool renderEdges;
	UPROPERTY(EditAnywhere, Category = "Debug Grid Display")
		bool renderNodes;
	UPROPERTY(EditAnywhere, Category = "Debug Grid Display")
		bool displayEdgeLengths;
#endif


protected:
	void calculateMesh();
	void rebuildInstances();
	UInstancedStaticMeshComponent* hexagonMeshComponent;
	UInstancedStaticMeshComponent* pentagonMeshComponent;
	USceneComponent* gridRoot;
	GridGenerator* gridGenerator;

#if WITH_EDITOR
	ULineBatchComponent* debugMesh;
	TArray<UTextRenderComponent*> debugTextArray;
	void buildDebugMesh();
	void rebuildDebugMesh();
	void updateDebugText();
#endif
};
