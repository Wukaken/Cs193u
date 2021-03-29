// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JBarrel.generated.h"

class UStaticMeshComponent;
class URadialForceComponent;
class UPrimitiveComponent;
class UParticleSystem;

UCLASS()
class CS193U_API AJBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJBarrel();

protected:
	// Called when the game starts or when spawned
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Explode")
	URadialForceComponent* ForceComp;

	UPROPERTY(EditAnywhere, Category = "Explode")
	UParticleSystem* ExplodeEffect;

	virtual void PostInitializeComponents() override;

public:	
	// Called every frame
	UFUNCTION()
	void OnActorHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
