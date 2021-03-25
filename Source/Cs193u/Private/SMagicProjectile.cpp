// Fill out your copyright notice in the Description page of Project Settings.


#include "SMagicProjectile.h"
#include "Components/SphereComponent.h"

ASMagicProjectile::ASMagicProjectile()
{
    SphereComp->SetSphereRadius(20.0f);
    //SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASMagicProjectile::OnActorOverlap);

    InitialLifeSpan = 10.0f;  // Derived from AActor
    DamageAmount = 20.0f;
}
