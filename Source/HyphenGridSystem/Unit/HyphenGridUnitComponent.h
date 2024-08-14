// Copyright Hyphen Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HyphenGridUnit.h"
#include "Components/ActorComponent.h"
#include "HyphenGridUnitComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HYPHENGRIDSYSTEM_API UHyphenGridUnitComponent : public UActorComponent, public IHyphenGridUnit
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHyphenGridUnitComponent();

	virtual FVector GetUnitLocation() override;
	virtual IHyphenGridCell* GetCurrentGridCell() override;
	virtual void SetCurrentGridCell(IHyphenGridCell* GridCell) override;
	virtual int32 GetUnitGroupID() override;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 UnitGroupID = 0;
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UObject> CurrentGridCell = nullptr;
};
