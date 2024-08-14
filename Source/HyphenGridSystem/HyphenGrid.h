// Copyright Hyphen Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HyphenGrid.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FHyphenGridInitializeData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GridSize = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridCount = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) // Offset is not implemented yet.
	FVector2D GridOffset = FVector2D::ZeroVector;
};

/**
 * 
 */
UCLASS()
class HYPHENGRIDSYSTEM_API UHyphenGrid : public UObject
{
	GENERATED_BODY()
};
