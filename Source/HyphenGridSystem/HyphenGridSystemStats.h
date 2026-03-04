// Copyright Hyphen Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"

// Stat group for HyphenGridSystem
// Project Settings → Session Frontend → Stat groups 에서 "HyphenGrid"로 표시됩니다.
DECLARE_STATS_GROUP(TEXT("HyphenGridSystem"), STATGROUP_HyphenGrid, STATCAT_Advanced);

// Cycle stats (scope-based timings)
DECLARE_CYCLE_STAT(TEXT("InitializeGridSystem"), STAT_Grid_Initialize, STATGROUP_HyphenGrid);
DECLARE_CYCLE_STAT(TEXT("RegisterGridUnit"),     STAT_Grid_RegisterUnit, STATGROUP_HyphenGrid);
DECLARE_CYCLE_STAT(TEXT("UnregisterGridUnit"),   STAT_Grid_UnregisterUnit, STATGROUP_HyphenGrid);
DECLARE_CYCLE_STAT(TEXT("OnGridUnitMove"),       STAT_Grid_OnGridUnitMove, STATGROUP_HyphenGrid);
DECLARE_CYCLE_STAT(TEXT("GetCellAtLocation"),    STAT_Grid_GetCellAtLocation, STATGROUP_HyphenGrid);
DECLARE_CYCLE_STAT(TEXT("GetCellsByLocation"),   STAT_Grid_GetCellsByLocation, STATGROUP_HyphenGrid);
DECLARE_CYCLE_STAT(TEXT("GetAdjacentGridCell"),  STAT_Grid_GetAdjacentGridCell, STATGROUP_HyphenGrid);
DECLARE_CYCLE_STAT(TEXT("GetGridUnitsByLocation"), STAT_Grid_GetGridUnitsByLocation, STATGROUP_HyphenGrid);
DECLARE_CYCLE_STAT(TEXT("GetGridUnitCount"),     STAT_Grid_GetGridUnitCount, STATGROUP_HyphenGrid);
DECLARE_CYCLE_STAT(TEXT("GetActorsInAABB"),      STAT_Grid_GetActorsInAABB, STATGROUP_HyphenGrid);

// Counters (integer metrics)
DECLARE_DWORD_COUNTER_STAT(TEXT("Units.Total"),      STAT_Grid_UnitsTotal, STATGROUP_HyphenGrid);
DECLARE_DWORD_COUNTER_STAT(TEXT("Buckets.Active"),   STAT_Grid_BucketsActive, STATGROUP_HyphenGrid);
DECLARE_DWORD_COUNTER_STAT(TEXT("Queries.Cells"),    STAT_Grid_QueriesCells, STATGROUP_HyphenGrid);
DECLARE_DWORD_COUNTER_STAT(TEXT("Queries.Units"),    STAT_Grid_QueriesUnits, STATGROUP_HyphenGrid);
