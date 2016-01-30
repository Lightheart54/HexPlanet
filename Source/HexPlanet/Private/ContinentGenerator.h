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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlates")
		int32 tectonicPlateSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlates")
		int32 numberOfPlateSeeds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlates")
		int32 numberOfSubPlateSeeds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlates")
		int32 addSubPlatesAfterIteration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing")
		int32 landMassingSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float percentOcean;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "0", UIMin = "0", ClampMax = "10", UIMax = "10"))
		int32 minPrimaryPlateTypeSeeds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "1", UIMin = "1", ClampMax = "10", UIMax = "10"))
		int32 primaryPlateTypeDensity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "0", UIMin = "0", ClampMax = "10", UIMax = "10"))
		int32 minSecondaryPlateTypeSeeds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "0", UIMin = "0", ClampMax = "10", UIMax = "10"))
		int32 secondaryPlateTypeDensity;

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void installGrid(AHexSphere* myOwner, GridGenerator* gridGenerator);
	
	void buildTectonicPlates();
	void calculateLandMasses();
	TArray<FGridTileSet> getPlateSets() const;
	FGridTileSet getOceanSet() const;
	FGridTileSet getLandSet() const;

	void createVoronoiDiagramFromSeedSets(GridTilePtrList &gridTiles, TArray<FGridTileSet>& seedSets, const uint32& maxNumIterations = -1);

	TArray<FGridTileSet> subdivideSetIntoSubgroups(const FGridTileSet& tileSet, const TArray<int32> subgroupSeedCount,
		const int32& groupSeed);
	bool addTileToTileSet(FGridTileSet& tileSet, const uint32& seedTile, GridTilePtrList& availableTiles);

	TArray<int32> getSetBorderTiles(const FGridTileSet& tileSet) const;
	void addTileSetToTileSet(FGridTileSet& set1, const FGridTileSet& set2);

protected:
	AHexSphere* gridOwner;
	GridGenerator* gridGen;
	
	TArray<FGridTileSet> plateTileSets; 
	TArray<FGridTileSet> subdividePlate(const FGridTileSet &plateSet, const int32& plateSeed);
	FGridTileSet landTileSet;
	FGridTileSet oceanTileSet;
};
