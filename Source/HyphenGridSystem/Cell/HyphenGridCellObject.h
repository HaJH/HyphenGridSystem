// Copyright Hyphen Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HyphenGridCell.h"
#include "UObject/Object.h"
#include "HyphenGridCellObject.generated.h"

/**
 * 
 */
UCLASS()
class HYPHENGRIDSYSTEM_API UHyphenGridCellObject : public UObject, public IHyphenGridCell
{
	GENERATED_BODY()
public:
	virtual void InitializeGridCell(const FHyphenGridCellData& Data) override;
	virtual void DrawCell() override;
	virtual const FHyphenGridCellData& GetCellData() override;
	virtual FVector GetCellCenterLocation() override;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FHyphenGridCellData CellData;
};
