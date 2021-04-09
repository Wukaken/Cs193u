// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SBTTask_RangedAttack.generated.h"

UCLASS()
class CS193U_API USBTTask_RangedAttack : public UBTTaskNode
{
	GENERATED_BODY()

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	// max random bullet spread(in degrees) in positive and negative angle(shared between yaw and pitch)
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxBulletSpread;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	TSubclassOf<AActor> ProjectileClass;

public:
	USBTTask_RangedAttack();
};
