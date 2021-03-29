// Fill out your copyright notice in the Description page of Project Settings.


#include "JProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AJProjectile::AJProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>("CollisionComp");
	CollisionComp->InitSphereRadius(5.f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AJProjectile::OnHit);

	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	RootComponent = CollisionComp;

	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
	MoveComp->UpdatedComponent = CollisionComp;
	MoveComp->InitialSpeed = 3000.f;
	MoveComp->MaxSpeed = 3000.f;
	MoveComp->bRotationFollowsVelocity = true;
	MoveComp->bShouldBounce = true;
}

USphereComponent* AJProjectile::GetCollisionComp() const
{
	return CollisionComp;
}
	
UProjectileMovementComponent* AJProjectile::GetMovementComp() const
{
	return MoveComp;
}

void AJProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
			   			 UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
			   			 const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("I am bullet being hit"));
	if(ensure(OtherActor) && OtherActor != this && ensure(OtherComp) && OtherComp->IsSimulatingPhysics()){
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.f, GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("Adding Impulse"));
	}
}