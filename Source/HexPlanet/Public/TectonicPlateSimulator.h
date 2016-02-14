// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "SphereGrid.h"
#include "GridMesher.h"
#include "TectonicPlateSimulator.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEXPLANET_API UTectonicPlateSimulator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTectonicPlateSimulator();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseMesh")
		UGridMesher* myMesher;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseMesh")
		USphereGrid* myGrid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseMesh")
		UMaterialInterface* overlayMaterial;

	void buildTectonicPlates();
	TArray<TArray<int32>> currentPlateSets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration")
		int32 plateSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration")
		int32 numBasePlates;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration")
		int32 numBaseSubplates;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration")
		int32 addSubplatesAfterNSteps;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float percentTilesForShapeReseed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float percentTilesForBorderReseed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration")
		bool showPlateOverlay;

	void generateInitialHeightMap();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InitialHeightMap")
		TArray<float> currentHeightMap; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InitialHeightMap")
		int32 heightMapSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InitialHeightMap")
		int32 numOctaves;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InitialHeightMap")
		bool showBaseHeightMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InitialHeightMap")
		TArray<FColor> elevationColorKey;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InitialHeightMap",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "100.0", UIMax = "100.0"))
		float percentOcean;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InitialHeightMap")
		bool showInitialContinents;


	void addNewSeedSetsToSetArray(TArray<bool> &usedTiles, TArray<TArray<int32>> &plateSets, const int32& numNewSets);

	int32 getNextAvailableSeedTile(TArray<bool> &usedTiles, TArray<TArray<int32>> & plateSets);

	void createVoronoiDiagramFromSeedSets(TArray<TArray<int32>>& seedSets, TArray<bool>& tileAvailability, const int32& maxNumIterations = -1);
	void rebuildTectonicPlate(TArray<TArray<int32>>& plateSets, const float& percentTilesForReseed);
	void meshTectonicPlateOverlay();
};
