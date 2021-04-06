// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SSaveGame.generated.h"

USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()
public:
	// Identifier for which actor this belongs to
	UPROPERTY()
	FString ActorName;

	// For movable actors, keep location, rotation, scale, etc
	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TArray<uint8> ByteData;
};

UCLASS()
class CS193U_API USSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:	
	UPROPERTY()
	int32 Credits;

	// longest survival time
	UPROPERTY()
	float PersonalRecordTime;

	UPROPERTY()
	TArray<FActorSaveData> SaveActors;
};
