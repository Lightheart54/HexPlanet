// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "SphereGrid.h"
#include "GridMesher.h"
#include "TectonicPlateSimulator.generated.h"

USTRUCT(BlueprintType)
struct FCrustCellData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
	FRectGridLocation gridLoc;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TectonicPlateSimulation")
	float cellHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
	float crustThickness;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
	float crustArea;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
	float crustDensity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
	int32 owningPlate;
};

USTRUCT(BlueprintType)
struct FTectonicPlate
{
	GENERATED_USTRUCT_BODY()
	//The index of this plate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
	int32 plateIndex;
	//The indexes of the crust cells that this plate owns
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
	TArray<int32> ownedCrustCells;
	//velocities in spherical coordinates and about the axis through the plate center of mass
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
	FVector currentVelocity;
	//crust cell about which the plate is centered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
	int32 centerOfMassIndex;
	
	//the maximum arc distance on the plate from its center
	//this is used to quickly determine which other plates it might overlap
	float plateBoundingRadius;
};

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

	UFUNCTION(BlueprintCallable, Category = "TectonicPlateSimulation")
	void generateInitialHeightMap();
	UFUNCTION(BlueprintPure, Category = "TectonicPlateSimulation")
	FCrustCellData createBaseCrustCell(const int32& cellIndex, const float& cellHeight) const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration")
		TArray<FCrustCellData> crustCells;		
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InitialHeightMap",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "100.0", UIMax = "100.0"))
		float percentContinentalCrust;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InitialHeightMap")
		bool showInitialContinents;

	UFUNCTION(BlueprintCallable, Category = "TectonicPlateSimulation")
	void buildTectonicPlates();
	UFUNCTION(BlueprintCallable, Category = "TectonicPlateSimulation")
	FTectonicPlate createTectonicPlate(const int32& plateIndex, const TArray<int32>& plateCellIndexes) const;
	UFUNCTION(BlueprintCallable, Category = "TectonicPlateSimulation")
	void updatePlateCenterOfMass(FTectonicPlate &newPlate) const;
	UFUNCTION(BlueprintCallable, Category = "TectonicPlateSimulation")
	void updatePlateBoundingRadius(FTectonicPlate& newPlate) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration")
		TArray<FTectonicPlate> currentPlates;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration")
		bool stopAfterFirstPlate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateGeneration")
		int32 plateToShowCenterOfMassDebugPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
		float lithosphereDensity; //kg/m^3
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
		float oceanicCrustDensity; //kg/m^3
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
		float continentalCrustDensity; //kg/m^3
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlateSimulation")
		float oceanicWaterDensity; //kg/m^3

protected:
	void addNewSeedSetsToSetArray(TArray<bool> &usedTiles, TArray<TArray<int32>> &plateSets, const int32& numNewSets);
	int32 getNextAvailableSeedTile(TArray<bool> &usedTiles, TArray<TArray<int32>> & plateSets);
	void createVoronoiDiagramFromSeedSets(TArray<TArray<int32>>& seedSets, TArray<bool>& tileAvailability, const int32& maxNumIterations = -1);
	void rebuildTectonicPlates(TArray<TArray<int32>>& plateSets, const float& percentTilesForReseed);
	void meshTectonicPlateOverlay();
};
