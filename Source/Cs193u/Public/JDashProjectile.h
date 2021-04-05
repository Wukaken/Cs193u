// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JDashProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UParticleSystem;
class UPrimitiveComponent;
class UProjectileMovementComponent;

UCLASS()
class CS193U_API AJDashProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJDashProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportDelay;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float DetonateDelay;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* MoveComp;

	// Handle to cancel timer if we already hit something
	FTimerHandle TimerHandle_DelayedDetonate;

	void Explode();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComp;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Fx")
	UParticleSystem* ImpactVFX;

	UFUNCTION()
	void OnActorHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
			   		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
			   		const FHitResult& Hit);
public:	
	void TeleportInstigator();

};
