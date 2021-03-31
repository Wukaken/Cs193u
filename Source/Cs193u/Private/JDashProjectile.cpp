// Fill out your copyright notice in the Description page of Project Settings.


#include "JDashProjectile.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AJDashProjectile::AJDashProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TeleportDelay = 0.2f;
	DetonateDelay = 0.2f;

	CollisionComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AJDashProjectile::OnActorHit);
	RootComponent = CollisionComp;

	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMoveComp");
	MoveComp->bRotationFollowsVelocity = true;
	MoveComp->bInitialVelocityInLocalSpace = true;
	MoveComp->ProjectileGravityScale = 0.f;
	MoveComp->InitialSpeed = 3000.f;

	InitialLifeSpan = 3.f;
}

// Called when the game starts or when spawned
void AJDashProjectile::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(TimerHandle_DelayedDetonate, this, &AJDashProjectile::Explode, DetonateDelay);
}	

void AJDashProjectile::Explode()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_DelayedDetonate);
	if(ensure(ImpactVFX)){
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation());
	}
	MoveComp->StopMovementImmediately();
	SetActorEnableCollision(false);

	FTimerHandle TimerHandle_DelayedTeleport;
	GetWorldTimerManager().SetTimer(TimerHandle_DelayedTeleport, this, &AJDashProjectile::TeleportInstigator, TeleportDelay);
}	

void AJDashProjectile::TeleportInstigator()
{
	AActor* ActorToTeleport = GetInstigator();
	if(ensure(ActorToTeleport))
		ActorToTeleport->TeleportTo(GetActorLocation(), ActorToTeleport->GetActorRotation(), false, false);
}

void AJDashProjectile::OnActorHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
			   					  UPrimitiveComponent* OtherComp, FVector NormalImpulse,
			   					  const FHitResult& Hit)
{
	Explode();
}
