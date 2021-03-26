// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction.h"
#include "SActionComponent.h"
#include "../Cs193u.h"
#include "Net/UnrealNetwork.h"

void USAction::Initialize(USActionComponent* NewActionComp)
{
    ActionComp = NewActionComp;
}

bool USAction::IsRunning() const
{
    return RepData.bIsRunning;
}

bool USAction::CanStart_Implementation(AActor* Instigator)
{
    // when Instigator is running, can not start
    if(IsRunning()){
        return false;
    }
    USActionComponent* Comp = GetOwningComponent();
    // if Action Component has any tag that in blog tags, can not start
    if(Comp->ActiveGameplayTags.HasAny(BlockedTags)){
        return false;
    }
    return true;
}

void USAction::StartAction_Implementation(AActor* Instigator)
{
    UE_LOG(LogTemp, Log, TEXT("Started: %s"), *GetNameSafe(this));

    // push all current grant tags to the ActionComponent
    USActionComponent* Comp = GetOwningComponent();
    Comp->ActiveGameplayTags.AppendTags(GrantsTags);

    RepData.bIsRunning = true;
    RepData.Instigator = Instigator;

    if(GetOwningComponent()->GetOwnerRole() == ROLE_Authority)
        TimeStarted = GetWorld()->TimeSeconds;

    // broadcasting here comes a new action component and start move time
    GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
}

void USAction::StopAction_Implementation(AActor* Instigator)
{
    UE_LOG(LogTemp, Log, TEXT("Stopped: %s"), *GetNameSafe(this));

    USActionComponent* Comp = GetOwningComponent();
    Comp->ActiveGameplayTags.RemoveTags(GrantsTags);

    RepData.bIsRunning = false;
    RepData.Instigator = Instigator;

    GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);
}

UWorld* USAction::GetWorld() const
{
    AActor* Actor = Cast<AActor>(GetOuter());
    if(Actor)
        return Actor->GetWorld();

    return nullptr;
}

bool USAction::IsSupportedForNetworking() const
{
    return true;
}

USActionComponent* USAction::GetOwningComponent() const
{
    //AActor* Actor = Cast<AActor>(GetOuter());
    //return Actor->GetComponentByClass(USActionComponent::StaticClass());
    return ActionComp;
}

void USAction::OnRep_RepData()
{
    if(RepData.bIsRunning)
        StartAction(RepData.Instigator);
    else
        StopAction(RepData.Instigator);
}

void USAction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USAction, RepData);
    DOREPLIFETIME(USAction, TimeStarted);
    DOREPLIFETIME(USAction, ActionComp);    
}