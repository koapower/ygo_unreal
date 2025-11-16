// YGO_unreal - Game UI Widget Implementation

#include "YGOGameWidget.h"
#include "YGOPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

void UYGOGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化快取值
	CachedLocalPlayerLP = 8000;
	CachedOpponentLP = 8000;
	CachedLocalPlayerHandCount = 0;

	// 自動尋找本地玩家的 PlayerState
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		LocalPlayerState = Cast<AYGOPlayerState>(PC->PlayerState);
		if (LocalPlayerState)
		{
			UE_LOG(LogTemp, Log, TEXT("[YGOGameWidget] Local PlayerState found and bound"));
		}
	}

	// 嘗試自動尋找對手 PlayerState（如果有的話）
	// 這個會在 Tick 中更新，因為對手可能稍後才加入遊戲
}

void UYGOGameWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 如果還沒有對手 PlayerState，嘗試尋找
	if (!OpponentPlayerState && LocalPlayerState)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			for (TActorIterator<AYGOPlayerState> It(World); It; ++It)
			{
				AYGOPlayerState* PS = *It;
				if (PS && PS != LocalPlayerState)
				{
					SetPlayerStates(LocalPlayerState, PS);
					UE_LOG(LogTemp, Log, TEXT("[YGOGameWidget] Opponent PlayerState found and bound"));
					break;
				}
			}
		}
	}

	// 檢查並更新手牌顯示
	CheckAndUpdateHandDisplay();
}

void UYGOGameWidget::SetPlayerStates(AYGOPlayerState* LocalPlayer, AYGOPlayerState* OpponentPlayer)
{
	// 解除舊的綁定
	UnbindPlayerStateEvents();

	// 設定新的玩家狀態
	LocalPlayerState = LocalPlayer;
	OpponentPlayerState = OpponentPlayer;

	// 綁定事件
	BindPlayerStateEvents();

	// 初始化顯示
	if (LocalPlayerState)
	{
		CachedLocalPlayerLP = LocalPlayerState->LifePoints;
		UpdateLocalPlayerLP_Implementation(LocalPlayerState->LifePoints, LocalPlayerState->LifePoints);
	}

	if (OpponentPlayerState)
	{
		CachedOpponentLP = OpponentPlayerState->LifePoints;
		UpdateOpponentLP_Implementation(OpponentPlayerState->LifePoints, OpponentPlayerState->LifePoints);
	}

	// 更新手牌顯示
	CheckAndUpdateHandDisplay();

	UE_LOG(LogTemp, Log, TEXT("[YGOGameWidget] PlayerStates set - Local: %s, Opponent: %s"),
		LocalPlayerState ? TEXT("Valid") : TEXT("NULL"),
		OpponentPlayerState ? TEXT("Valid") : TEXT("NULL"));
}

void UYGOGameWidget::BindPlayerStateEvents()
{
	if (LocalPlayerState)
	{
		// 綁定本地玩家 LP 改變事件
		LocalPlayerState->OnLifePointsChanged.AddDynamic(this, &UYGOGameWidget::HandleLocalPlayerLPChanged);

		// 綁定本地玩家抽牌事件
		LocalPlayerState->OnCardDrawn.AddDynamic(this, &UYGOGameWidget::HandleLocalPlayerCardDrawn);
	}

	if (OpponentPlayerState)
	{
		// 綁定對手 LP 改變事件
		OpponentPlayerState->OnLifePointsChanged.AddDynamic(this, &UYGOGameWidget::HandleOpponentLPChanged);

		// 綁定對手手牌數量改變事件
		OpponentPlayerState->OnHandCountChanged.AddDynamic(this, &UYGOGameWidget::HandleOpponentHandCountChanged);
	}
}

void UYGOGameWidget::UnbindPlayerStateEvents()
{
	if (LocalPlayerState)
	{
		LocalPlayerState->OnLifePointsChanged.RemoveDynamic(this, &UYGOGameWidget::HandleLocalPlayerLPChanged);
		LocalPlayerState->OnCardDrawn.RemoveDynamic(this, &UYGOGameWidget::HandleLocalPlayerCardDrawn);
	}

	if (OpponentPlayerState)
	{
		OpponentPlayerState->OnLifePointsChanged.RemoveDynamic(this, &UYGOGameWidget::HandleOpponentLPChanged);
		OpponentPlayerState->OnHandCountChanged.RemoveDynamic(this, &UYGOGameWidget::HandleOpponentHandCountChanged);
	}
}

// ========================================================================
// 手牌顯示
// ========================================================================

void UYGOGameWidget::UpdateHandDisplay_Implementation(const TArray<FYGOCardInstance>& HandCards)
{
	// 預設實作 - 觸發 Blueprint 事件
	OnHandChanged(HandCards);
}

void UYGOGameWidget::UpdateOpponentHandDisplay_Implementation(int32 HandCount)
{
	// 預設實作 - 觸發 Blueprint 事件
	OnOpponentHandCountChanged(HandCount);
}

TArray<FYGOCardInstance> UYGOGameWidget::GetLocalPlayerHand() const
{
	if (LocalPlayerState)
	{
		return LocalPlayerState->ClientHand;
	}
	return TArray<FYGOCardInstance>();
}

int32 UYGOGameWidget::GetOpponentHandCount() const
{
	if (OpponentPlayerState)
	{
		return OpponentPlayerState->HandCount;
	}
	return 0;
}

void UYGOGameWidget::CheckAndUpdateHandDisplay()
{
	if (LocalPlayerState)
	{
		int32 CurrentHandCount = LocalPlayerState->ClientHand.Num();
		if (CurrentHandCount != CachedLocalPlayerHandCount)
		{
			CachedLocalPlayerHandCount = CurrentHandCount;
			UpdateHandDisplay_Implementation(LocalPlayerState->ClientHand);
		}
	}
}

// ========================================================================
// LP 顯示
// ========================================================================

void UYGOGameWidget::UpdateLocalPlayerLP_Implementation(int32 OldLP, int32 NewLP)
{
	// 預設實作 - 觸發 Blueprint 事件
	OnLocalPlayerLPChanged(OldLP, NewLP);
	CachedLocalPlayerLP = NewLP;
}

void UYGOGameWidget::UpdateOpponentLP_Implementation(int32 OldLP, int32 NewLP)
{
	// 預設實作 - 觸發 Blueprint 事件
	OnOpponentLPChanged(OldLP, NewLP);
	CachedOpponentLP = NewLP;
}

int32 UYGOGameWidget::GetLocalPlayerLP() const
{
	if (LocalPlayerState)
	{
		return LocalPlayerState->LifePoints;
	}
	return 0;
}

int32 UYGOGameWidget::GetOpponentLP() const
{
	if (OpponentPlayerState)
	{
		return OpponentPlayerState->LifePoints;
	}
	return 0;
}

// ========================================================================
// 其他遊戲資訊
// ========================================================================

int32 UYGOGameWidget::GetLocalPlayerDeckCount() const
{
	if (LocalPlayerState)
	{
		return LocalPlayerState->DeckCount;
	}
	return 0;
}

int32 UYGOGameWidget::GetOpponentDeckCount() const
{
	if (OpponentPlayerState)
	{
		return OpponentPlayerState->DeckCount;
	}
	return 0;
}

int32 UYGOGameWidget::GetLocalPlayerExtraDeckCount() const
{
	if (LocalPlayerState)
	{
		return LocalPlayerState->ExtraDeckCount;
	}
	return 0;
}

int32 UYGOGameWidget::GetOpponentExtraDeckCount() const
{
	if (OpponentPlayerState)
	{
		return OpponentPlayerState->ExtraDeckCount;
	}
	return 0;
}

int32 UYGOGameWidget::GetLocalPlayerGraveyardCount() const
{
	if (LocalPlayerState)
	{
		return LocalPlayerState->Graveyard.Num();
	}
	return 0;
}

int32 UYGOGameWidget::GetOpponentGraveyardCount() const
{
	if (OpponentPlayerState)
	{
		return OpponentPlayerState->Graveyard.Num();
	}
	return 0;
}

// ========================================================================
// 事件處理
// ========================================================================

void UYGOGameWidget::HandleLocalPlayerLPChanged(int32 OldLP, int32 NewLP)
{
	UpdateLocalPlayerLP_Implementation(OldLP, NewLP);
}

void UYGOGameWidget::HandleOpponentLPChanged(int32 OldLP, int32 NewLP)
{
	UpdateOpponentLP_Implementation(OldLP, NewLP);
}

void UYGOGameWidget::HandleLocalPlayerCardDrawn(const FYGOCardInstance& DrawnCard)
{
	// 觸發 Blueprint 事件
	OnCardDrawn(DrawnCard);

	// 更新手牌顯示
	UpdateHandDisplay_Implementation(LocalPlayerState->ClientHand);
}

void UYGOGameWidget::HandleOpponentHandCountChanged(int32 HandCount) {
	UpdateOpponentHandDisplay_Implementation(HandCount);
}
