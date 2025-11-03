#include "CardBase.h"

ACardBase::ACardBase()
{
	// PrimaryActorTick.bCanEverTick = true;
}

bool ACardBase::IsNetRelevantFor(const AActor *RealViewer, const AActor *ViewTarget, const FVector &SrcLocation) const
{
	// 呼叫 Blueprint 可實作版本
	return BP_IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

bool ACardBase::BP_IsNetRelevantFor_Implementation(const AActor *RealViewer, const AActor *ViewTarget, const FVector &SrcLocation) const
{
	// 預設行為：全部 relevant
	return true;
}
