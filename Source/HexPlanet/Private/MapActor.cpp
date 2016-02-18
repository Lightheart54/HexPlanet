// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "MapActor.h"


// Sets default values
AMapActor::AMapActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	gridMesher = CreateDefaultSubobject<UGridMesher>(TEXT("MapMesh"));
	sphereGrid = CreateDefaultSubobject<USphereGrid>(TEXT("MapGrid"));
	plateSimul = CreateDefaultSubobject<UTectonicPlateSimulator>(TEXT("PlateSimulator"));
	RootComponent = gridMesher;
	gridMesher->myGrid = sphereGrid;
	plateSimul->myGrid = sphereGrid;
	plateSimul->myMesher = gridMesher;
	framesPerRotation = 2000;
}

// Called when the game starts or when spawned
void AMapActor::BeginPlay()
{
	Super::BeginPlay();
	gridMesher->rebuildBaseMeshFromGrid();
	plateSimul->generateInitialHeightMap();
	plateSimul->buildTectonicPlates();
}

// Called every frame
void AMapActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	//rotate the sphere
	if (framesPerRotation > 0)
	{
		FRotator currentRotation = GetActorRotation();
		currentRotation.Yaw += 360.0 / framesPerRotation;
		SetActorRotation(currentRotation);
	}

}

