// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPrimitiveComponent;

UCLASS()
class CS193U_API AJProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJProjectile();

protected:
	// Called when the game starts or when spawned
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* MoveComp;

public:	
	// Called every frame
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
			   UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
			   const FHitResult& Hit);

	USphereComponent* GetCollisionComp() const;
	
	UProjectileMovementComponent* GetMovementComp() const;
};
