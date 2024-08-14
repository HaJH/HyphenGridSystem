// Copyright Hyphen Games, Inc. All Rights Reserved.


#include "HyphenGridUnitComponent.h"

#include "HyphenGridSystem/Cell/HyphenGridCell.h"
#include "HyphenGridSystem/Manager/HyphenGridManager.h"


// Sets default values for this component's properties
UHyphenGridUnitComponent::UHyphenGridUnitComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

FVector UHyphenGridUnitComponent::GetUnitLocation()
{
	return GetOwner()->GetActorLocation();
}

IHyphenGridCell* UHyphenGridUnitComponent::GetCurrentGridCell()
{
	return Cast<IHyphenGridCell>(CurrentGridCell);
}

void UHyphenGridUnitComponent::SetCurrentGridCell(IHyphenGridCell* GridCell)
{
	CurrentGridCell = Cast<UObject>(GridCell);
}

int32 UHyphenGridUnitComponent::GetUnitGroupID()
{
	return UnitGroupID;
}

void UHyphenGridUnitComponent::BeginPlay()
{
	Super::BeginPlay();
	// Register Grid Unit to Grid Manager
	if(IsValid(AHyphenGridManager::Get()))
	{
		AHyphenGridManager::Get()->RegisterGridUnit(this);
	}
}

void UHyphenGridUnitComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister Grid Unit from Grid Manager
	if(IsValid(AHyphenGridManager::Get()))
	{
		AHyphenGridManager::Get()->UnregisterGridUnit(this);
	}
	Super::EndPlay(EndPlayReason);
}
