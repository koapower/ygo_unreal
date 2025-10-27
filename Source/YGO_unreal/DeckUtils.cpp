// Fill out your copyright notice in the Description page of Project Settings.


#include "DeckUtils.h"

void UDeckUtils::ShuffleIntArray(TArray<int32>& Array)
{
    const int32 LastIndex = Array.Num() - 1;
    for (int32 i = LastIndex; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        Array.Swap(i, j);
    }
}