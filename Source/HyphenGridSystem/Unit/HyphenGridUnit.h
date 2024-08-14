// Copyright Hyphen Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HyphenGridUnit.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHyphenGridUnit : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HYPHENGRIDSYSTEM_API IHyphenGridUnit
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void SetCurrentGridCell(class IHyphenGridCell* GridCell) = 0;
	virtual class IHyphenGridCell* GetCurrentGridCell() = 0;
	virtual FVector GetUnitLocation() = 0;
	virtual int32 GetUnitGroupID() = 0;
};
