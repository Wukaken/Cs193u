// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SAttributeComponent.h"
#include "SGameModeBase.h"
#include "Net/UnrealNetwork.h"

static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("su.DamageMultiplier"), 1.f, TEXT("Global Damage Modifier for Attribute Component."), ECVF_Cheat);

USAttributeComponent::USAttributeComponent()
{
	HealthMax = 100;
	Health = HealthMax;

	Rage = 0;
	RageMax = 100;
	SetIsReplicatedByDefault(true);
}

bool USAttributeComponent::Kill(AActor* InstigatorActor)
{
	return ApplyHealthChange(InstigatorActor, -GetHealthMax());
}

bool USAttributeComponent::IsAlive() const
{
	return Health > 0.f;
}

bool USAttributeComponent::IsFullHealth() const
{
	return Health == HealthMax;
}

float USAttributeComponent::GetHealth() const
{
	return Health;
}

float USAttributeComponent::GetHealthMax() const
{
	return HealthMax;
}

bool USAttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	if(!GetOwner()->CanBeDamaged() && Delta < 0.f)
		return false;
	
	if(Delta < 0.f){
		float DamageMultiplier = CVarDamageMultiplier.GetValueOnGameThread();
		Delta *= DamageMultiplier;
	}

	float OldHealth = Health;
	float NewHealth = FMath::Clamp(Health + Delta, 0.f, HealthMax);

	float ActualDelta = NewHealth - OldHealth;
	// Is Server
	if(GetOwner()->HasAuthority()){
		Health = NewHealth;
		if(ActualDelta != 0.f)
			MulticastHealthChanged(InstigatorActor, Health, ActualDelta);

		// Died
		if(ActualDelta < 0.f && Health == 0.f){
			ASGameModeBase* GM = GetWorld()->GetAuthGameMode<ASGameModeBase>();
			if(GM)
				GM->OnActorKilled(GetOwner(), InstigatorActor);
		}
	}

	return ActualDelta != 0;
}

float USAttributeComponent::GetRage() const
{
	return Rage;
}

bool USAttributeComponent::ApplyRage(AActor* InstigatorActor, float Delta)
{
	float OldRage = Rage;
	Rage = FMath::Clamp(Rage + Delta, 0.f, RageMax);
	float ActualDelta = Rage - OldRage;
	if(ActualDelta != 0.f)
		OnRageChanged.Broadcast(InstigatorActor, this, Rage, ActualDelta);

	return ActualRage != 0;
}

USAttributeComponent* USAttributeComponent::GetAttributes(AActor* FromActor)
{
	if(FromActor)
		return Cast<USAttributeComponent>(FromActor->GetComponentByClass(USAttributeComponent::StaticClass()));

	return nullptr;
}

bool USAttributeComponent::IsActorAlive(AActor* Actor)
{
	USAttributeComponent* AttributeComp = GetAttributes(Actor);
	if(AttributeComp)
		return AttributeComp->IsAlive();

	return false;
}

void USAttributeComponent::MulticastHealthChanged(AActor* InstigatorActor, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(Instigator, this, NewHealth, Delta);
}

void USAttributeComponent::MulticastRageChanged(AActor* InstigatorActor, float NewHealth, float Delta)
{
	OnRageChanged.Broadcast(Instigator, this, NewRage, Delta);
}

void USAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// all replicated property should be DOREPLIFETIME
	DOREPLIFETIME(USAttributeComponent, Health);
	DOREPLIFETIME(USAttributeComponent, HealthMax);

	DOREPLIFETIME(USAttributeComponent, Rage);
	DOREPLIFETIME(USAttributeComponent, RageMax);
}