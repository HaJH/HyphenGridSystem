// Copyright Hyphen Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HyphenGridSettings.generated.h"

/**
 * 
 */
UCLASS(config=Game, defaultconfig)
class HYPHENGRIDSYSTEM_API UHyphenGridSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	static FORCEINLINE UHyphenGridSettings* Get()
	{
		UHyphenGridSettings* Settings = GetMutableDefault<UHyphenGridSettings>();
		check(Settings);
		return Settings;
	}
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "HyphenGridSystem|Avoidance")
	int32 GridCount = 10;
};
