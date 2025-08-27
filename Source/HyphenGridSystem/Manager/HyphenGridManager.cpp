// Copyright Hyphen Games, Inc. All Rights Reserved.


#include "HyphenGridManager.h"

#include "HyphenGridSystem/Cell/HyphenGridCell.h"
#include "HyphenGridSystem/Cell/HyphenGridCellObject.h"

#include "HyphenUtilLibrary.h"
#include "HyphenGridSystem/HyphenGridSettings.h"
#include "HyphenGridSystem/Unit/HyphenGridUnit.h"
#include "HyphenGridSystem/HyphenGridSystemStats.h"
#include "DrawDebugHelpers.h"

TObjectPtr<AHyphenGridManager> AHyphenGridManager::Instance = nullptr;

// Sets default values
AHyphenGridManager::AHyphenGridManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = .5f;
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

void AHyphenGridManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Allow runtime toggle of debug setting
	const bool bDebug = UHyphenGridSettings::Get()->bDebug;
	if (!bDebug)
	{
		if (IsActorTickEnabled())
		{
			SetActorTickEnabled(false);
		}
		return;
	}
	else if (!IsActorTickEnabled())
	{
		SetActorTickEnabled(true);
	}
	
	DebugDraw();
}

void AHyphenGridManager::InitializeGridSystem(const FHyphenGridInitializeData& InitializeData)
{
	SCOPE_CYCLE_COUNTER(STAT_Grid_Initialize);
	GridInitializeData = InitializeData;

	// Cache grid math constants
	CachedGridCount = GridInitializeData.GridCount;
	CachedCellSize = (CachedGridCount > 0) ? (GridInitializeData.GridSize / CachedGridCount) : 0.f;
	InvCellSize = (CachedCellSize > 0.f) ? (1.f / CachedCellSize) : 0.f;
	HalfGridSize = GridInitializeData.GridSize * 0.5f;

	// Initialize Grid System (-GridSize/2, -GridSize/2) ~ (GridSize/2, GridSize/2) with GridCount * GridCount
	for(int32 i = 0; i < CachedGridCount; ++i)
	{
		for(int32 j = 0; j < CachedGridCount; ++j)
		{
			const FVector CellLocation = FVector(CachedCellSize * (i - CachedGridCount / 2), CachedCellSize * (j - CachedGridCount / 2), 0);
			FHyphenGridCellData CellData;
			CellData.Location = CellLocation;
			CellData.Size = CachedCellSize;
			CellData.Index = i * CachedGridCount + j;
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
	SCOPE_CYCLE_COUNTER(STAT_Grid_RegisterUnit);
	AActor* Actor = HyphenUtil::GetInterfaceActor(GridUnit);
	if(Actor)
	{
		AllGridUnits.Add(Actor);
		INC_DWORD_STAT(STAT_Grid_UnitsTotal);
		// Add Grid Unit to Grid Cell by Location
		FVector Location = GridUnit->GetUnitLocation();
		IHyphenGridCell* GridCell = GetCellAtLocation(Location);
		if(GridCell)
		{
			if (FGridUnitSet* Bucket = GridUnitsByGridCell.Find(Cast<UObject>(GridCell)))
			{
				Bucket->GridUnitSet.Add(Actor);
			}
			else
			{
				FGridUnitSet NewSet; NewSet.GridUnitSet.Add(Actor);
				GridUnitsByGridCell.Add(Cast<UObject>(GridCell), MoveTemp(NewSet));
				INC_DWORD_STAT(STAT_Grid_BucketsActive);
			}
			GridUnit->SetCurrentGridCell(GridCell);
		}
	}
}

void AHyphenGridManager::UnregisterGridUnit(IHyphenGridUnit* GridUnit)
{
	SCOPE_CYCLE_COUNTER(STAT_Grid_UnregisterUnit);
	if(IsValid(Get()) == false)
	{
		return;
	}
	AActor* Actor = HyphenUtil::GetInterfaceActor(GridUnit);
	if(Actor && AllGridUnits.Contains(Actor))
	{
		AllGridUnits.Remove(Actor);
		DEC_DWORD_STAT(STAT_Grid_UnitsTotal);
		// Remove Grid Unit from Grid Cell by Location
		if(IHyphenGridCell* GridCell = GridUnit->GetCurrentGridCell())
		{
			if (FGridUnitSet* Bucket = GridUnitsByGridCell.Find(Cast<UObject>(GridCell)))
			{
				Bucket->GridUnitSet.Remove(Actor);
				if (Bucket->GridUnitSet.Num() == 0)
				{
					GridUnitsByGridCell.Remove(Cast<UObject>(GridCell));
					DEC_DWORD_STAT(STAT_Grid_BucketsActive);
				}
			}
			GridUnit->SetCurrentGridCell(nullptr);
		}
	}
}

void AHyphenGridManager::OnGridUnitMove(IHyphenGridUnit* GridUnit, FVector NewLocation)
{
	SCOPE_CYCLE_COUNTER(STAT_Grid_OnGridUnitMove);
	AActor* Actor = HyphenUtil::GetInterfaceActor(GridUnit);
	if(Actor && AllGridUnits.Contains(Actor))
	{
		IHyphenGridCell* OldCell = GridUnit->GetCurrentGridCell();
		IHyphenGridCell* NewCell = GetCellAtLocation(NewLocation);
		if(OldCell == NewCell)
		{
			return;
		}

		// Remove from old bucket
		if(OldCell)
		{
			if (FGridUnitSet* Bucket = GridUnitsByGridCell.Find(Cast<UObject>(OldCell)))
			{
				Bucket->GridUnitSet.Remove(Actor);
				if (Bucket->GridUnitSet.Num() == 0)
				{
					GridUnitsByGridCell.Remove(Cast<UObject>(OldCell));
				}
			}
			GridUnit->SetCurrentGridCell(nullptr);
		}
		// Add to new bucket
		if(NewCell)
		{
			if (FGridUnitSet* NewBucket = GridUnitsByGridCell.Find(Cast<UObject>(NewCell)))
			{
				NewBucket->GridUnitSet.Add(Actor);
			}
			else
			{
				FGridUnitSet NewSet; NewSet.GridUnitSet.Add(Actor);
				GridUnitsByGridCell.Add(Cast<UObject>(NewCell), MoveTemp(NewSet));
				INC_DWORD_STAT(STAT_Grid_BucketsActive);
			}
			GridUnit->SetCurrentGridCell(NewCell);
		}
	}
}

IHyphenGridCell* AHyphenGridManager::GetCellAtLocation(FVector Location)
{
	SCOPE_CYCLE_COUNTER(STAT_Grid_GetCellAtLocation);
	// Fast index computation using cached values
	if (CachedGridCount <= 0 || CachedCellSize <= 0.f)
	{
		return nullptr;
	}
	const float XLocal = (Location.X + HalfGridSize) * InvCellSize; // in cell units
	const float YLocal = (Location.Y + HalfGridSize) * InvCellSize;
	int32 GridX = FMath::FloorToInt(XLocal);
	int32 GridY = FMath::FloorToInt(YLocal);
	// Unsigned range check catches negative and overflow in single compare
	if ((uint32)GridX >= (uint32)CachedGridCount || (uint32)GridY >= (uint32)CachedGridCount)
	{
		return nullptr;
	}
	const int32 Index = GridX * CachedGridCount + GridY;
	return GridCells.IsValidIndex(Index) ? Cast<IHyphenGridCell>(GridCells[Index]) : nullptr;
}

TArray<IHyphenGridCell*> AHyphenGridManager::GetCellsByLocation(FVector Location, float Radius)
{
	SCOPE_CYCLE_COUNTER(STAT_Grid_GetCellsByLocation);
	INC_DWORD_STAT(STAT_Grid_QueriesCells);
	TArray<IHyphenGridCell*> CellsInRadius;

	if (CachedGridCount <= 0 || CachedCellSize <= 0.f)
	{
		return CellsInRadius;
	}

	// Compute candidate index range via world-space circle bounds
	const float R = FMath::Max(Radius, 0.f);
	const int32 MinX = FMath::FloorToInt(((Location.X - R) + HalfGridSize) * InvCellSize);
	const int32 MaxX = FMath::FloorToInt(((Location.X + R) + HalfGridSize) * InvCellSize);
	const int32 MinY = FMath::FloorToInt(((Location.Y - R) + HalfGridSize) * InvCellSize);
	const int32 MaxY = FMath::FloorToInt(((Location.Y + R) + HalfGridSize) * InvCellSize);

	const int32 ClampedMinX = FMath::Clamp(MinX, 0, CachedGridCount - 1);
	const int32 ClampedMaxX = FMath::Clamp(MaxX, 0, CachedGridCount - 1);
	const int32 ClampedMinY = FMath::Clamp(MinY, 0, CachedGridCount - 1);
	const int32 ClampedMaxY = FMath::Clamp(MaxY, 0, CachedGridCount - 1);

	const float R2 = R * R;
	for (int32 i = ClampedMinX; i <= ClampedMaxX; ++i)
	{
		for (int32 j = ClampedMinY; j <= ClampedMaxY; ++j)
		{
			const int32 Index = i * CachedGridCount + j;
			if (!GridCells.IsValidIndex(Index)) continue;

			// Compute AABB of cell in world
			const float MinCX = CachedCellSize * (i - CachedGridCount / 2);
			const float MinCY = CachedCellSize * (j - CachedGridCount / 2);
			const float MaxCX = MinCX + CachedCellSize;
			const float MaxCY = MinCY + CachedCellSize;

			// Closest point on AABB to circle center
			const float Qx = FMath::Clamp(Location.X, MinCX, MaxCX);
			const float Qy = FMath::Clamp(Location.Y, MinCY, MaxCY);
			const float Dx = Qx - Location.X;
			const float Dy = Qy - Location.Y;
			if (Dx*Dx + Dy*Dy <= R2)
			{
				CellsInRadius.Add(Cast<IHyphenGridCell>(GridCells[Index]));
			}
		}
	}
	return CellsInRadius;
}

TArray<IHyphenGridCell*> AHyphenGridManager::GetAdjacentGridCell(IHyphenGridCell* GridCell)
{
	SCOPE_CYCLE_COUNTER(STAT_Grid_GetAdjacentGridCell);
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
		if (FGridUnitSet* Bucket = GridUnitsByGridCell.Find(Cast<UObject>(GridCell)))
		{
			return Bucket->GridUnitSet;
		}
	}
	return EmptyGridUnitSet.GridUnitSet;
}

TArray<IHyphenGridUnit*> AHyphenGridManager::GetGridUnitsByLocation(FVector Location, float Radius)
{
	SCOPE_CYCLE_COUNTER(STAT_Grid_GetGridUnitsByLocation);
	INC_DWORD_STAT(STAT_Grid_QueriesUnits);
	const auto& Cells = GetCellsByLocation(Location, Radius);
	TArray<IHyphenGridUnit*> GridUnits;
	const float R2 = Radius * Radius;
	for(IHyphenGridCell* Cell : Cells)
	{
		const TSet<TWeakObjectPtr<UObject>>& Bucket = GetGridUnits(Cell);
		for (const TWeakObjectPtr<UObject>& Obj : Bucket)
		{
			if (IHyphenGridUnit* Unit = Cast<IHyphenGridUnit>(Obj))
			{
				if ((Unit->GetUnitLocation() - Location).SizeSquared() <= R2)
				{
					GridUnits.Add(Unit);
				}
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
	SCOPE_CYCLE_COUNTER(STAT_Grid_GetGridUnitCount);
	if (FGridUnitSet* Bucket = GridUnitsByGridCell.Find(Cast<UObject>(GridCell)))
	{
		return Bucket->GridUnitSet.Num();
	}
	return 0;
}

void AHyphenGridManager::DebugDraw()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CachedGridCount <= 0 || CachedCellSize <= 0.f)
	{
		return;
	}

	// Draw grid bounds (slightly above ground)
	const float Z = 50.f;
	const FVector Origin(0.f, 0.f, Z);
	const float Half = HalfGridSize;
	const FVector A(-Half, -Half, Z);
	const FVector B( Half, -Half, Z);
	const FVector C( Half,  Half, Z);
	const FVector D(-Half,  Half, Z);
	const float Duration = PrimaryActorTick.TickInterval * 1.01;
	const float Thickness = 10.f;
	DrawDebugLine(World, A, B, FColor::Cyan, false, Duration, 0, Thickness);
	DrawDebugLine(World, B, C, FColor::Cyan, false, Duration, 0, Thickness);
	DrawDebugLine(World, C, D, FColor::Cyan, false, Duration, 0, Thickness);
	DrawDebugLine(World, D, A, FColor::Cyan, false, Duration, 0, Thickness);

	// Draw each cell box and info
	for (int32 i = 0; i < CachedGridCount; ++i)
	{
		for (int32 j = 0; j < CachedGridCount; ++j)
		{
			const int32 Index = i * CachedGridCount + j;
			if (!GridCells.IsValidIndex(Index)) continue;
			IHyphenGridCell* Cell = Cast<IHyphenGridCell>(GridCells[Index]);
			if (!Cell) continue;

			const FVector Center = Cell->GetCellCenterLocation() + FVector(0,0,Z);
			const FVector Extents(CachedCellSize * 0.5f, CachedCellSize * 0.5f, 2.f);
			const int32 UnitCount = GetGridUnitCount(Cell);
			FColor Color = FColor::Green;
			if (UnitCount > 0) Color = FColor::Yellow;
			if (UnitCount > 3) Color = FColor::Red;

			DrawDebugBox(World, Center, Extents, FQuat::Identity, Color, false, Duration, 0, 4.f);

			// Show indices and unit count
			if (UnitCount > 0)
			{
				FString Text = FString::Printf(TEXT("(%d,%d) #%d"), i, j, UnitCount);
				DrawDebugString(World, Center + FVector(0,0,25.f), Text, nullptr, FColor::White, Duration, false, 1.f);
			}
		}
	}
}
