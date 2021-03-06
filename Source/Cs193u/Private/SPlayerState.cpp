// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SPlayerState.h"
#include "SSaveGame.h"
#include "Net/UnrealNetwork.h"

void ASPlayerState::AddCredits(int32 Delta)
{
	// Avoid user-error of adding a negative amount
	if(!ensure(Delta >= 0.f))
		return;

	Credits += Delta;
	OnCreditsChanged.Broadcast(this, Credits, Delta);
}

bool ASPlayerState::RemoveCredits(int32 Delta)
{
	// Avoid user-error of adding a subtracting negative amount
	if(!ensure(Delta >= 0.f))
		return false;

	// not enough credits available
	if(Credits < Delta)
		return false;
	
	Credits -= Delta;
	OnCreditsChanged.Broadcast(this, Credits, -Delta);
	return true;
}

bool ASPlayerState::UpdatePersonalRecord(float NewTime)
{
	//Higher time is better
	if(NewTime > PersonalRecordTime){
		float OldRecord = PersonalRecordTime;
		PersonalRecordTime = NewTime;
		OnRecordTimeChanged.Broadcast(this, PersonalRecordTime, OldRecord);

		return true;
	}
	return false;
}

void ASPlayerState::SavePlayerState_Implementation(USSaveGame* SaveObject)
{
	if(SaveObject){
		SaveObject->Credits = Credits;
		SaveObject->PersonalRecordTime = PersonalRecordTime;
	}
}

void ASPlayerState::LoadPlayerState_Implementation(USSaveGame* SaveObject)
{
	if(SaveObject){
		//Credits = SaveObject->Credits;
		//Make sure we trigger credits changed event
		AddCredits(SaveObject->Credits);

		PersonalRecordTime = SaveObject->PersonalRecordTime;
	}
}

void ASPlayerState::OnRep_Credits(int32 OldCredits)
{
	OnCreditsChanged.Broadcast(this, Credits, Credits - OldCredits);
}

/*
void ASPlayerState::MulticastCredits_Implementation(float NewCredits, float Delta)
{
	OnCreditsChanged.Broadcast(this, NewCredits, Delta);
}
*/

int32 ASPlayerState::GetCredits() const
{
	return Credits;
}

void ASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASPlayerState, Credits);
}
