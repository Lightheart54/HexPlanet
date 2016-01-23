// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "HexSphere.h"
#include "grid/grid.h"


// Sets default values
AHexSphere::AHexSphere()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Grid trialGrid(20, 4);
}

// Called when the game starts or when spawned
void AHexSphere::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHexSphere::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

