// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPowerupActor.h"
#include "SPowerup_HealthPotion.generated.h"

class UStaticMeshComponent;

UCLASS()
class CS193U_API ASPowerup_HealthPotion : public ASPowerupActor
{
	GENERATED_BODY()
	
protected:	
	UPROPERTY(EditAnywhere = "HealthPotion")
	int32 CreditCost;
public:
	void Interact_Implementation(APawn* InstigatorPawn) override;

	FText GetInteractText_Implementation(APawn* InstigatorPawn) override;

	ASPowerup_HealthPotion();
};