// Copyright Hyphen Games, Inc. All Rights Reserved.


#include "HyphenGridCellObject.h"

void UHyphenGridCellObject::InitializeGridCell(const FHyphenGridCellData& Data)
{
	CellData = Data;
	// DrawCell();
}

void UHyphenGridCellObject::DrawCell()
{
	// Debug Draw
	DrawDebugBox(GetWorld(), CellData.Location + FVector(CellData.Size / 2, CellData.Size / 2, 0),
	             FVector(CellData.Size / 2, CellData.Size / 2, 1), FColor::Red, false, 2.f);
}

const FHyphenGridCellData& UHyphenGridCellObject::GetCellData()
{
	return CellData;
}

FVector UHyphenGridCellObject::GetCellCenterLocation()
{
	return CellData.Location + FVector(CellData.Size / 2, CellData.Size / 2, 0);
}
