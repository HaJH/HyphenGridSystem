// Copyright Hyphen Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HyphenGridCell.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FHyphenGridCellData
{
	GENERATED_BODY()
	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	UPROPERTY()
	float Size = 0;
	UPROPERTY()
	int Index = INDEX_NONE;
	UPROPERTY()
	int GridXIndex = INDEX_NONE;
	UPROPERTY()
	int GridYIndex = INDEX_NONE;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHyphenGridCell : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HYPHENGRIDSYSTEM_API IHyphenGridCell
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void InitializeGridCell(const FHyphenGridCellData& Data) = 0;
	virtual void DrawCell() = 0;

	virtual FVector GetCellCenterLocation() = 0;
	virtual const FHyphenGridCellData& GetCellData() = 0;
};
