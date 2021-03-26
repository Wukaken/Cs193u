// Fill out your copyright notice in the Description page of Project Settings.


#include "JProjectile.h"

// Sets default values
AJProjectile::AJProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AJProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AJProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

