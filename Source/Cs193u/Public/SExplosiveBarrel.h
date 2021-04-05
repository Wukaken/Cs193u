// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMagicProjectile.generated.h"

class UStaticMeshComponent;
class URadialForceComponent;
/**
 * 
 */
UCLASS()
class CS193U_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere)
	URadialForceComponent* ForceComp;

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	void OnActorHit(UPrimitiveComponent* HitComponent, 
					AActor* OtherActor, UPrimitiveComponent* OtherComp, 
					FVector NormalImpulse, const FHitResult& Hit);
public:
	ASExplosiveBarrel();
};
