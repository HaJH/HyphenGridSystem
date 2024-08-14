// Copyright Hyphen Games, Inc. All Rights Reserved.


#include "HyphenGridManager.h"

#include "HyphenGridSystem/Cell/HyphenGridCell.h"
#include "HyphenGridSystem/Cell/HyphenGridCellObject.h"

#include "HyphenUtilLibrary.h"
#include "HyphenGridSystem/HyphenGridSettings.h"
#include "HyphenGridSystem/Unit/HyphenGridUnit.h"

TObjectPtr<AHyphenGridManager> AHyphenGridManager::Instance = nullptr;

// Sets default values
AHyphenGridManager::AHyphenGridManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

AHyphenGridManager* AHyphenGridManager::Get()
{
	return Instance.Get();
}

// Called when the game starts or when spawned
void AHyphenGridManager::BeginPlay()
{
	Super::BeginPlay();
	Instance = this;
}

void AHyphenGridManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Instance = nullptr;
	Super::EndPlay(EndPlayReason);
}


void AHyphenGridManager::InitializeGridSystem(const FHyphenGridInitializeData& InitializeData)
{
	GridInitializeData = InitializeData;
	// Initialize Grid System (-GridSize/2, -GridSize/2) ~ (GridSize/2, GridSize/2) with GridCount * GridCount
	for(int32 i = 0; i < InitializeData.GridCount; ++i)
	{
		for(int32 j = 0; j < InitializeData.GridCount; ++j)
		{
			float CellSize = InitializeData.GridSize / InitializeData.GridCount;
			FVector CellLocation = FVector(CellSize * (i - InitializeData.GridCount / 2), CellSize * (j - InitializeData.GridCount / 2), 0);
			FHyphenGridCellData CellData;
			CellData.Location = CellLocation;
			CellData.Size = CellSize;
			CellData.Index = i * InitializeData.GridCount + j;
			CellData.GridXIndex = i;
			CellData.GridYIndex = j;
			// Create Grid Cell
			UHyphenGridCellObject* GridCell = NewObject<UHyphenGridCellObject>(this);
			GridCell->InitializeGridCell(CellData);
			// Register Grid Cell
			GridCells.Add(GridCell);
		}
	}
}

void AHyphenGridManager::RegisterGridUnit(IHyphenGridUnit* GridUnit)
{
	AActor* Actor = HyphenUtil::GetInterfaceActor(GridUnit);
	if(Actor)
	{
		AllGridUnits.Add(Actor);
		// Add Grid Unit to Grid Cell by Location
		FVector Location = GridUnit->GetUnitLocation();
		IHyphenGridCell* GridCell = GetCellAtLocation(Location);
		if(GridCell)
		{
			GridUnitsByGridCell.FindOrAdd(Cast<UObject>(GridCell)).GridUnitSet.Add(Actor);
			GridUnit->SetCurrentGridCell(GridCell);
		}
	}
}

void AHyphenGridManager::UnregisterGridUnit(IHyphenGridUnit* GridUnit)
{
	if(IsValid(Get()) == false)
	{
		return;
	}
	AActor* Actor = HyphenUtil::GetInterfaceActor(GridUnit);
	if(Actor && AllGridUnits.Contains(Actor))
	{
		AllGridUnits.Remove(Actor);
		// Remove Grid Unit from Grid Cell by Location
		if(IHyphenGridCell* GridCell = GridUnit->GetCurrentGridCell())
		{
			GridUnitsByGridCell.FindOrAdd(Cast<UObject>(GridCell)).GridUnitSet.Remove(Actor);
			GridUnit->SetCurrentGridCell(nullptr);
		}
	}
}

void AHyphenGridManager::OnGridUnitMove(IHyphenGridUnit* GridUnit, FVector NewLocation)
{
	AActor* Actor = HyphenUtil::GetInterfaceActor(GridUnit);
	if(Actor && AllGridUnits.Contains(Actor))
	{
		if(GridUnit->GetCurrentGridCell() != nullptr)
		{
			if(GetCellAtLocation(NewLocation) == GridUnit->GetCurrentGridCell())
			{
				return;
			}
		}
		
		// Remove Grid Unit from Grid Cell by Location
		if(IHyphenGridCell* GridCell = GridUnit->GetCurrentGridCell())
		{
			GridUnitsByGridCell.FindOrAdd(Cast<UObject>(GridCell)).GridUnitSet.Remove(Actor);
			GridUnit->SetCurrentGridCell(nullptr);
		}
		// Add Grid Unit to Grid Cell by Location
		IHyphenGridCell* NewGridCell = GetCellAtLocation(NewLocation);
		if(NewGridCell)
		{
			GridUnitsByGridCell.FindOrAdd(Cast<UObject>(NewGridCell)).GridUnitSet.Add(Actor);
			GridUnit->SetCurrentGridCell(NewGridCell);
		}
	}
}

IHyphenGridCell* AHyphenGridManager::GetCellAtLocation(FVector Location)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AHyphenGridManager_GetCellAtLocation);
	// Calculate grid indices based on location and cell size
	int32 GridX = FMath::FloorToInt((Location.X + GridInitializeData.GridSize / 2) / (GridInitializeData.GridSize / GridInitializeData.GridCount));
	int32 GridY = FMath::FloorToInt((Location.Y + GridInitializeData.GridSize / 2) / (GridInitializeData.GridSize / GridInitializeData.GridCount));

	// Ensure the calculated indices are within the grid boundaries
	if (GridX < 0 || GridX >= GridInitializeData.GridCount || GridY < 0 || GridY >= GridInitializeData.GridCount)
	{
		return nullptr; // Return -1 or another invalid value to indicate out-of-bounds
	}

	// Convert 2D grid indices to 1D index
	int32 Index = GridX * GridInitializeData.GridCount + GridY;
	if(GridCells.IsValidIndex(Index))
	{
		return Cast<IHyphenGridCell>(GridCells[Index]);
	}
	else
	{
		return nullptr;
	}
}

TArray<IHyphenGridCell*> AHyphenGridManager::GetCellsByLocation(FVector Location, float Radius)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AHyphenGridManager_GetCellsByLocation);
	TArray<IHyphenGridCell*> CellsInRadius;

	// Find the grid cell index of the given location
	auto* CenterCell = GetCellAtLocation(Location);
	if(CenterCell == nullptr)
	{
		return CellsInRadius;
	}
	int32 CenterIndex = CenterCell->GetCellData().Index;
	
	// Calculate the number of cells in the radius
	float CellSize = GridInitializeData.GridSize / GridInitializeData.GridCount;
	int32 CellsInRadiusCount = FMath::CeilToInt(Radius / CellSize);

	// Convert center index to 2D grid coordinates
	int32 CenterX = CenterIndex / GridInitializeData.GridCount;
	int32 CenterY = CenterIndex % GridInitializeData.GridCount;

	// Loop through the cells in the radius
	for (int32 i = CenterX - CellsInRadiusCount; i <= CenterX + CellsInRadiusCount; ++i)
	{
		for (int32 j = CenterY - CellsInRadiusCount; j <= CenterY + CellsInRadiusCount; ++j)
		{
			// Check grid boundaries
			if (i >= 0 && i < GridInitializeData.GridCount && j >= 0 && j < GridInitializeData.GridCount)
			{
				// Convert 2D grid coordinates back to 1D index
				int32 index = i * GridInitializeData.GridCount + j;
				auto* Cell = Cast<IHyphenGridCell>(GridCells[index]);
				CellsInRadius.Add(Cell);
			}
		}
	}
	return CellsInRadius;
}

TArray<IHyphenGridCell*> AHyphenGridManager::GetAdjacentGridCell(IHyphenGridCell* GridCell)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AHyphenGridManager_GetAdjacentGridCell);
	// Get around grid cells
	TArray<IHyphenGridCell*> AroundGridCells;
	if(GridCell)
	{
		int32 GridX = GridCell->GetCellData().GridXIndex;
		int32 GridY = GridCell->GetCellData().GridYIndex;
		for(int32 i = GridX - 1; i <= GridX + 1; ++i)
		{
			for(int32 j = GridY - 1; j <= GridY + 1; ++j)
			{
				if(i == GridX && j == GridY)
				{
					continue;
				}
				if(i < 0 || i >= GridInitializeData.GridCount || j < 0 || j >= GridInitializeData.GridCount)
				{
					continue;
				}
				int32 Index = i * GridInitializeData.GridCount + j;
				if(GridCells.IsValidIndex(Index))
				{
					AroundGridCells.Add(Cast<IHyphenGridCell>(GridCells[Index]));
				}
			}
		}
	}
	return AroundGridCells;
}

const TSet<TWeakObjectPtr<UObject>>& AHyphenGridManager::GetGridUnits(IHyphenGridCell* GridCell)
{
	if(GridCell)
	{
		return GridUnitsByGridCell.FindOrAdd(Cast<UObject>(GridCell)).GridUnitSet;
	}
	return EmptyGridUnitSet.GridUnitSet;
}

TArray<IHyphenGridUnit*> AHyphenGridManager::GetGridUnitsByLocation(FVector Location, float Radius)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AHyphenGridManager_GetGridUnitsByLocation);
	const auto& Cells = GetCellsByLocation(Location, Radius);
	TSet<TWeakObjectPtr<UObject>> GridUnitSet;
	for(IHyphenGridCell* Cell : Cells)
	{
		GridUnitSet.Append(GetGridUnits(Cell));
	}
	TArray<IHyphenGridUnit*> GridUnits;
	for(const TWeakObjectPtr<UObject>& GridUnit : GridUnitSet)
	{
		// Check if the unit distance is within the radius
		if(IHyphenGridUnit* Unit = Cast<IHyphenGridUnit>(GridUnit))
		{
			if((Unit->GetUnitLocation() - Location).SizeSquared() <= Radius * Radius)
			{
				GridUnits.Add(Unit);
			}
		}
	}
	return GridUnits;
}


// FVector AHyphenGridManager::GetAdjustedLocation(FVector Location, FVector DesiredLocation, float Coefficient)
// {
// 	QUICK_SCOPE_CYCLE_COUNTER(STAT_AHyphenGridManager_GetAdjustedLocation);
// 	IHyphenGridCell* GridCell = GetCellAtLocation(Location);
// 	if(GridCell == nullptr)
// 	{
// 		return Location;
// 	}
//
// 	const int32 GridCellUnitCount = GetGridUnitCount(GridCell);
// 	// If Grid Cell has no Grid Unit, return location
// 	if(GridCellUnitCount == 0)
// 	{
// 		return Location;
// 	}
// 	// If Grid Cell has more than one Grid Unit, Calculate location that avoid center of Grid Cell
// 	FVector CenterLocation = GridCell->GetCellCenterLocation();
// 	// Get Adjust Cells
// 	const auto& AdjacentCells = GetAdjacentGridCell(GridCell);
// 	// Find the cell that is the most sparse
// 	IHyphenGridCell* SparseCell = nullptr;
// 	int32 SparseCellUnitCount = 0;
// 	float SparseCellDistanceSquared = 0;
// 	for(IHyphenGridCell* AdjacentCell : AdjacentCells)
// 	{
// 		if(AdjacentCell == nullptr)
// 		{
// 			continue;
// 		}
// 		float AdjacentCellDistanceSquared = (AdjacentCell->GetCellCenterLocation() - DesiredLocation).SizeSquared();
// 		int32 AdjacentCellUnitCount = GetGridUnitCount(AdjacentCell);
// 		if(SparseCell == nullptr || SparseCellUnitCount > AdjacentCellUnitCount || (SparseCellUnitCount == AdjacentCellUnitCount && AdjacentCellDistanceSquared < SparseCellDistanceSquared))
// 		{
// 			SparseCell = AdjacentCell;
// 			SparseCellUnitCount = AdjacentCellUnitCount;
// 			SparseCellDistanceSquared = (AdjacentCell->GetCellCenterLocation() - DesiredLocation).SizeSquared();
// 		}
// 	}
// 	// Calculate out vector from center location to adjacent sparse grid cell location
// 	if(SparseCell)
// 	{
// 		// Calculate out vector from center location to adjacent sparse grid cell location
// 		const int32 CellUnitLimit = UHyphenGridSettings::Get()->GridCellUnitLimit;
// 		// const float GridCellCountPower = GridCellUnitCount * GridCellUnitCount;
// 		float AvoidancePower = (float)GridCellUnitCount / (float)CellUnitLimit;
// 		AvoidancePower *= Coefficient;
// 		AvoidancePower = FMath::Clamp(AvoidancePower, 0.f, 1.f);
// 		FVector AdjustVector = SparseCell->GetCellCenterLocation() - CenterLocation;
// 		AdjustVector.Normalize();
// 		AdjustVector *= AvoidancePower;
// 		AdjustVector *= GridInitializeData.GridSize / GridInitializeData.GridCount;
// 		// Draw Debug Arrow
// 		DrawDebugDirectionalArrow(GetWorld(), CenterLocation + FVector(0,0,300), CenterLocation + AdjustVector + FVector(0,0,300), 100.f, FColor::Green, false, 1.f);
// 		return Location + AdjustVector;
// 	}
// 	return Location;
// }

int32 AHyphenGridManager::GetGridUnitCount(IHyphenGridCell* GridCell)
{
	return GridUnitsByGridCell.FindOrAdd(Cast<UObject>(GridCell)).GridUnitSet.Num();
}
