// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileBase.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/EngineTypes.h"

// Sets default values
ASProjectileBase::ASProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Projectile");
	SphereComp->OnComponentHit.AddDynamic(this, &ASProjectileBase::OnActorHit);
	RootComponent = SphereComp;

	EffectComp = CreateDefaultSubobject<UParticleSystemComponent>("EffectComp");
	EffectComp->SetupAttachment(RootComponent);

	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMoveComp");
	MoveComp->bRotationFollowsVelocity = true;
	MoveComp->bInitialVelocityInLocalSpace = true;
	MoveComp->ProjectileGravityScale = 0.0f;
	MoveComp->InitialSpeed = 8000;

	SetReplicates(true);
}

void ASProjectileBase::OnActorHit(UPrimitiveComponent* HitComponent, 
								  AActor* OtherActor, UPrimitiveComponent* OtherComp, 
								  FVector NormalImpulse, const FHitResult& Hit)
{
	Explode();
}

void ASProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// _Implementation from it being marked as BlueprintNativeEvent
void ASProjectileBase::Explode_Implementation()
{
	if(ensure(!IsPendingKill())){
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation());
		Destroy();
	}
}
