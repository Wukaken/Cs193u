// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameModeBase.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "AI/SAICharacter.h"
#include "SAttributeComponent.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "SCharacter.h"
#include "SPlayerState.h"
#include "SSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "SGameplayInterface.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "SMonsterData.h"
#include "../Cs193u.h"
#include "SActionComponent.h"
#include "Engine/AssetManager.h"

static TAutoConsoleVariable<bool> CVarSpawnBots(TEXT("su.SpawnBots"), true, TEXT("Enable spawning of bots via timer"), ECVF_Cheat);

ASGameModeBase::ASGameModeBase()
{
	SpawnTimerInterval = 2.f;
	CreditsPerKill = 20;
	CooldownTimeBetweenFailures = 8.f;

	DesiredPowerupCount = 10;
	RequiredPowerupDistance = 2000;
	
	PlayerStateClass = ASPlayerState::StaticClass();

	SlotName = "SaveGame01";
}

void ASGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	FString SelectedSaveSlot = UGameplayStatics::ParseOption(Options, "SaveGame");
	if(SelectedSaveSlot.Len() > 0)
		SlotName = SelectedSaveSlot;

	LoadSaveGame();
}

void ASGameModeBase::StartPlay()
{
	Super::StartPlay();
	//Continous timer to spawn in more bots
	// Actual amount of bots and whether its allowed to spawn determined by spawn logic later in the chain
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this, &ASCharacter::SpawnBotTimerElapsed, SpawnTimerInterval, true);
	
	// Make sure we have assigned at least one power-up class
	if(ensure(PowerClasses.Num() > 0)){
		// Run EQS to find potential power-up spawn locations
		UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, PowerupSpawnQuery, this, EEnvQueryRunMode::AllMatching, nullptr);
		if(ensure(QueryInstance))
			QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnPowerupSpawnQueryCompleted);
	}
}

void ASGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Calling before Super:: so we set variables before 'beginplayingstate' is called in PlayerController(which is where we instantiate UI)
	ASPlayerState* PS = NewPlayer->GetPlayerState<ASPlayerState>();
	if(ensure(PS))
		PS->LoadPlayerState(CurrentSaveGame);

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void ASGameModeBase::KillAll()
{
	for(TActorIterator<ASAICharacter> It(GetWorld()); It; ++It){
		ASAICharacter* Bot = *It;
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(Bot);
		if(ensure(AttributeComp) && AttributeComp->IsActive())
			AttributeComp->Kill(this); // @fixme: pass in player? for kill credit
	}
}

void ASGameModeBase::SpawnBotTimerElapsed()
{
	if(!CVarSpawnBots.GetValueOnGameThread())
		return;

	// give points to spend
	if(SpawnCreditCurve)
		AvailableSpawnCredit += SpawnCreditCurve->GetFloatValue(GetWorld()->TimeSeconds);

	if(CooldownBotSpawnUntil > GetWorld()->TimeSeconds)
	{
		// still Cooling down
		return;
	}

	LogOnScreen(this, FString::Printf("Available SpawnCredits: %f"), AvailableSpawnCredit);
	// Count alive bots before spawning
	int32 NrOfAlivebots = 0;
	for(TActorIterator<ASAICharacter> It(GetWorld()); It; It++){
		ASAICharacter* Bot = *It;
	}
}