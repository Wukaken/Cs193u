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
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(Bot);
		if(ensure(AttributeComp) && AttributeComp->IsAlive())
			NrOfAlivebots++;
	}

	UE_LOG(LogTemp, Log, TEXT("Found %i alive bots."), NrOfAliveBots);

	const float MaxBotCount = 40.f;
	if(NrOfAlivebots >= MaxBotCount){
		UE_LOG(LogTemp, Log, TEXT("At Maximum bot capacity. Skipping bot spawn."));
		return;
	}

	if(MonsterTable){
		//Reset before selecting new row
		SelectedMonsterRow = nullptr;
		TArray<FMonsterInfoRow*> Rows;
		MonsterTable->GetAllRows("", Rows);

		// Get Total weight
		float TotalWeight = 0;
		for(FMonsterInfoRow* Entry : Rows)
			TotalWeight += Entry->Weight;

		// Random number within total random
		int32 RandomWeight = FMath::RandRange(0.f, TotalWeight);
		// Reset
		TotalWeight = 0;
		// Get monster based on random weight
		for(FMonsterInfoRow* Entry : Rows){
			TotalWeight += Entry->Weight;
			if(RandomWeight <= TotalWeight){
				SelectedMonsterRow = Entry;
				break;
			}
		}

		if(SelectedMonsterRow && SelectedMonsterRow->SpawnCost >= AvailableSpawnCredit){
			// Too expensive to spawn, try again soon
			CooldownBotSpawnUntil = GetWorld()->TimeSeconds + CooldownTimeBetweenFailures;
			LogOnScreen(this, FString::Printf(TEXT("Cooling down until: %f"), CooldownBotSpawnUntil), FColor::Red);
			return;
		}
	}
	//Run EQS to find valid spawn location
	UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnBotQuery, this, EEnvQueryRunMode::RandomBest5Pct, nullptr);
	if(ensure(QueryInstance))
		QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnBotSpawnQueryCompleted);
}

void ASGameModeBase::OnBotSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QuertStatus)
{
	if(QueryStatus != EEnvQueryStatus::Success){
		UE_LOG(LogTemp, Warning, TEXT("Spawn bot EQS Query Failed!"));
		return;
	}

	TArray<FVector> Locations = QueryInstance->GetResultsAsLocations();
	if(Locations.IsValidIndex(0) && MonsterTable){
		if(UAssetManager* Manager = UAssetManager::GetIfValid()){
			// Apply spawn const
			AvailableSpawnCredit -= SelectedMonsterRow->SpawnCost;
			FPrimaryAssetId MonsterId = SelectedMonsterRow->MonsterId;

			TArray<FName> Bundles;
			FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ASGameMode::OnMonsterLoaded, MonsterId, Locations[0]);
			Manager->LoadPrimaryAsset(MonsterId, Bundles, Delegate);
		}
	}
}

void ASGameModeBase::OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation)
{
	// LogOnScreen(this, "Finish loading", FColor::Green);
	UAssetManager* Manager = UAssetManager::GetIfValid();
	if(ManagerE){
		USMonsterData* MonsterData = Cast<USMonsterData>(Manager->GetPrimaryAssetObject(LoadedId));
		if(MonsterData){
			AActor* NewBot = GetWorld()->SpawnActor<AActor>(MonsterData->MonsterClass, SpawnLocation, FRotator::ZeroRotator);
			if(NewBot){
				LogOnScreen(this, FString::Printf(TEXT("Spawn enemy: %s (%s)"), *GetNameSafe(NewBot), *GetNameSafe(MonsterData)));
				// Grant special actions, buffs etc.
				USActionComponent* ActionComp = Cast<USActionComponent>(NewBot->GetComponentByClass(USActionComponent::StaticClass()));
				if(ActionComp){
					for(TSubclassOf<USAction> ActionClass : MonsterData->Actions)
						ActionComp->AddAction(NewBot, ActionClass);
				}
			}
		}
	}
}

void ASGameModeBase::OnPowerupSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	if(QueryStatus != EEnvQueryStatus::Success){
		UE_LOG(LogTemp, Warning, TEXT("Spawn bot EQS Failed"));
		return;
	}

	TArray<FVector> Locations = QueryInstance->GetResultsAsLocations();
	// keep used locations to easily check distance between points
	TArray<FVector> UsedLocations;
	int32 SpawnCounter = 0;
	//break out if we reached the desired count or if we have no more potential positions remaining
	while(SpawnCounter <  DesiredPowerupCount && Locations.Num() > 0){
		// Pick a random location from remaining points
		int32 RandomLocationIndex = FMath::RandRange(0, Locations.Num() - 1);
		FVector PickedLocation = Locations[RandomLocationIndex];
		//remove to avoid picking again
		Locations.RemoveAt(RandomLocationIndex);

		// check minimum distance requirement
		bool bValidLocation = true;
		for(FVector OtherLocation : UsedLocations){
			float DistanceTo = (PickedLocation - OtherLocation).Size();
			if(DistanceTo < RequiredPowerupDistance){
				// Show Skip locations due to distance
				// DrawDebugSphere(GetWorld(), PickedLocation, 50.f, 20, FColor::Red, false, 10.f);
				// to close, skip to next attempt
				bValidLocation = false;
				break;
			}
		}
		// failed the distance test
		if(!bValidLocation)
			continue;
		// pick a random powerup-class
		int32 RandomClassIndex = FMath::RandRange(0, PowerupClasses.Num() - 1);
		TSubclassOf<AActor> RandomPowerupClass = PowerupClasses[RandomClassIndex];

		GetWorld()->SpawnActor<AActor>(RandomPowerupClass, PickedLocation, FRotator::ZeroRotator);

		// Keep for distance checks;
		UsedLocations.Add(PickedLocation);
		SpawnCounter++;
	}
}

void ASGameModeBase::RespawnPlayerElapsed(AController* Controller)
{
	if(ensure(Controller)){
		Controller->UnPossess();
		RestartPlayer(Controller);
	}
}

void ASGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("OnActorKilled: Victim: %s, Killer: %s"), *GetNameSafe(VictimActor), *GameNameSafe(Killer));
	// handle player death
	ASCharacter* Player = Cast<ASCharacter>(VictimActor);
	if(Player){
		// Disable auto-respawn
		// Store time if it was better than previous record
		ASPlayerState* PS = Player->GetPlayerState<ASPlayerState>();
		if(PS)
			PS->UpdatePersonalRecord(GetWorld()->TimeSeconds);

		// immediately auto save on death
		WriteSaveGame();
	}
	// Give Credits for kill
	APawn* KillerPawn = Cast<APawn>(Killer);
	// don't credit kills of self
	if(KillerPawn && KillerPawn != VictimActor){
		// Only Player will have a PlayerState instance, bots have nullptr
		ASPlayerState* PS = Player->GetPlayerState<ASPlayerState>();
		if(PS)
			PS->AddCredits(CreditsPerKill);
	}
}

void ASGameModeBase::WriteSaveGame()
{
	// iterate all player states, we don't have properID to match yet(requires Steam or EOS)
	for(int32 i = 0; i < GameState->PlayerArray.Num(); i++){
		ASPlayerState* PS = Cast<ASPlayerState>(GameState->PlayerArray[i]);
		if(PS){
			PS->SavePlayerState(CurrentSaveGame);
			break;
		}
	}
	CurrentSaveGame->SaveActors.Empty();

	// Iterate the entire world of actors
	for(FActorIterator It(GetWorld()); It; It++){
		AActor* Actor = *It;
		// only interested in our gameplay actors
		if(!Actor->Implements<USGameplayInterface>())
			continue;

		FActorSaveData ActorData;
		ActorData.ActorName = Actor->GetName();
		ActorData.Transform = Actor->GetActorTransform();

		// pass the array to fill with data from actor
		FMemoryWriter MemWriter(ActorData.ByteData);
		FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
		// Find only variables with UPROPERTY(SaveGame)}
		Ar.ArIsSaveGame = true;
		// Converts actor's savegame UPROPERTIES into binary array
		Actor->Serialize(Ar);
		CurrentSaveGame->SaveActors.Add(ActorData);
	}
	UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SlotName, 0);
}

void ASGameModeBase::LoadSaveGame()
{
	if(UGameplayStatics::DoesSaveGameExist(SlotName, 0)){
		CurrentSaveGame = Cast<USSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if(CurrentSaveGame == nullptr){
			UE_LOG(LogTemp, Warning, TEXT("Failed to load SaveGame Data."));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("Loaded SaveGame Data"));

		//Iterate the entire world of actors
		for(FActorIterator It(GetWorld()); It; It++){
			AActor* Actor = *It;
			// only interested in our gameplay actors
			if(!Actor->Implements<USGameplayInterface>())
				continue;
			for(FActorSaveData ActorData : CurrentSaveGame->SaveActors){
				if(ActorData.ActorName == Actor->GetName()){
					Actor->SetActorTransform(ActorData.Transform);
					FMemoryReader MemReader(ActorData.ByteData);
					FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
					Ar.ArIsSaveGame = true;
					// convert binary array back into actor's variables
					Actor->Serialize(Ar);
					ISGameplayInterace::Execute_OnActorLoaded(Actor);
					break;
				}
			}
		}
	}
	else{
		CurrentSaveGame = Cast<USSaveGame>(UGameplayStatics::CreateSaveGameObject(USSaveGame::StaticClass()));
		UE_LOG(LogTemp, Log, TEXT("Created New SaveGame Data."));
	}
}