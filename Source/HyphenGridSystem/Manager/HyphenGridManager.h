// Copyright Hyphen Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HyphenGridSystem/HyphenGrid.h"
#include "HyphenGridManager.generated.h"

class IHyphenGridCell;
class IHyphenGridUnit;

USTRUCT()
struct FGridUnitSet
{
	GENERATED_BODY()
	TSet<TWeakObjectPtr<UObject>> GridUnitSet;	
};

UCLASS()
class HYPHENGRIDSYSTEM_API AHyphenGridManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHyphenGridManager();
	static AHyphenGridManager* Get();

	void InitializeGridSystem(const FHyphenGridInitializeData& InitializeData);
	void RegisterGridUnit(IHyphenGridUnit* GridUnit);
	void UnregisterGridUnit(IHyphenGridUnit* GridUnit);
	void OnGridUnitMove(IHyphenGridUnit* GridUnit, FVector NewLocation);
	IHyphenGridCell* GetCellAtLocation(FVector Location);
	TArray<IHyphenGridCell*> GetCellsByLocation(FVector Location, float Radius);
	// IHyphenGridCell* GetCurrentGridCell(IHyphenGridUnit* GridUnit);
	TArray<IHyphenGridCell*> GetAdjacentGridCell(IHyphenGridCell* GridCell);
	// TArray<IHyphenGridCell*> GetAroundGridCell(IHyphenGridUnit* GridUnit);
	const TSet<TWeakObjectPtr<UObject>>& GetGridUnits(IHyphenGridCell* GridCell);
	// TArray<IHyphenGridUnit*> GetAroundGridUnits(IHyphenGridUnit* GridUnit);
	// TArray<IHyphenGridUnit*> GetSameGridUnits(IHyphenGridUnit* GridUnit);
	TArray<IHyphenGridUnit*> GetGridUnitsByLocation(FVector Location, float Radius);

	
	// FVector GetAdjustedLocation(FVector Location, FVector DesiredLocation, float Coefficient = 1.f);
	int32 GetGridUnitCount(IHyphenGridCell* GridCell);
	// FVector GetAvoidanceForce();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	// GridCells
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<class UObject>> GridCells;
	// GridUnits
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSet<TObjectPtr<class UObject>> AllGridUnits;
	// Grid Units by Grid Cell
	UPROPERTY()
	TMap<TObjectPtr<UObject>, FGridUnitSet> GridUnitsByGridCell;

	UPROPERTY()
	FHyphenGridInitializeData GridInitializeData;
	
	FGridUnitSet EmptyGridUnitSet;

private:
	static TObjectPtr<AHyphenGridManager> Instance;
};
