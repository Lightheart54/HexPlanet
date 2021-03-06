// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "SphereGrid.generated.h"

/*!
* \struct FRectGridIndex
* \brief A position in the Rectilinear Grid
*/
USTRUCT(BlueprintType)
struct FRectGridIndex
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 uPos;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 vPos;
};

/*!
* \struct FRectGridLocation
* \brief FRectGridLocation details the information about a particular location on the grid
*/
USTRUCT(BlueprintType)
struct FRectGridLocation
{
	GENERATED_USTRUCT_BODY()

	/*! The Unique Index For This Tile*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 tileIndex;
	/*! This Tile's Position(s) on the grid*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FRectGridIndex> gridPositions;
};

/*!
* \class USphereGrid
* \brief Actor Component For Building and Navigating the Grid
* \details The Sphere Grid is responsible for building the grid
* Delivering the tiles, and responsible for computing neighbors,
* Manhattan distances between points, and translating between a 
* spherical location and the corresponding tile.
* \note the axial grid coordinate system for the produced grid
* is such that u - v + w = 0
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEXPLANET_API USphereGrid : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USphereGrid();

	// Called when the game starts
	virtual void BeginPlay() override;


	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Properties",
		meta = (ClampMin = "1", UIMin = "1", ClampMax = "1000", UIMax = "1000"))
		int32 gridFrequency;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Properties")
		int32 numNodes;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Properties")
		float icosahedronInteriorAngle;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	FRectGridLocation mapPosToTile(const FVector& positionOnSphere) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	int32 mapPosToTileIndex(FVector positionOnSphere, ULineBatchComponent* debugOut = nullptr, float debugRadius = 200.0) const;

	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	FVector getNodeLocationOnSphereUV(const int32& uLoc, const int32& vLoc) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	FVector getNodeLocationOnSphere(const FRectGridLocation& gridTile) const;

	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<FRectGridLocation> getLocationsForIndexes(const TArray<int32>& locationIndexs) const;

	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<FRectGridLocation> getTileNeighbors(const FRectGridLocation& gridTile) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<int32> getTileNeighborIndexes(const FRectGridLocation& gridTile) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<FRectGridLocation> getTilesNStepsAway(const FRectGridLocation& gridTile, const int32& numSteps) const; 
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<int32> getTileIndexesNStepsAway(const FRectGridLocation& gridTile, const int32& numSteps) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<int32> getIndexNeighbors(const FRectGridIndex& gridIndex) const;

	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	void decrementU(int32 &nextU, int32 &nextV) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	void incrementU(int32 &nextU, int32 &nextV) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<FRectGridLocation> getStraightPathBetweenTiles(const FRectGridLocation& startTile, const FRectGridLocation& endTile) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<int32> getStraightIndexPathBetweenTiles(const FRectGridLocation& startTile, const FRectGridLocation& endTile) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	void expandTileSet(TArray<int32>& tileIndexSet, TArray<bool>& tileAvailability) const;

	/*! The Raw Grid */
	TArray<TArray<int32>> rectilinearGridM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Properties")
	TArray<FRectGridLocation> gridLocationsM;

	TMap<int32, FVector> gridReferencePointsM;

protected:

	void addTileToNeighborList(int32 nextU, int32 nextV, TArray<int32> &tilesInRange, int32& nextTileIndex) const;

	TArray<FVector> createBaseIcosahedron();
	FVector projectVectorOntoIcosahedronFace(const FVector& positionOnSphere, const FVector& refPoint, const FVector& uDir, const FVector& vDir) const;
};
