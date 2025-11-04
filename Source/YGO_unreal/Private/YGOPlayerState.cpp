// YGO_unreal - Player State Implementation

#include "YGOPlayerState.h"
#include "Net/UnrealNetwork.h"

AYGOPlayerState::AYGOPlayerState()
{
	YGOPlayerID = 0;
	LifePoints = 8000;
	DeckCount = 0;
	ExtraDeckCount = 0;
	HandCount = 0;
	bHasNormalSummoned = false;
	NormalSummonCount = 0;
	bHasEnteredBattlePhase = false;
	NextInstanceID = 1;

	// 啟用網路複製
	bReplicates = true;
	SetReplicates(true);
}

void AYGOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYGOPlayerState, YGOPlayerID);
	DOREPLIFETIME(AYGOPlayerState, LifePoints);
	DOREPLIFETIME(AYGOPlayerState, Graveyard);
	DOREPLIFETIME(AYGOPlayerState, Banished);
	DOREPLIFETIME(AYGOPlayerState, DeckCount);
	DOREPLIFETIME(AYGOPlayerState, ExtraDeckCount);
	DOREPLIFETIME(AYGOPlayerState, HandCount);
	DOREPLIFETIME(AYGOPlayerState, bHasNormalSummoned);
	DOREPLIFETIME(AYGOPlayerState, NormalSummonCount);
	DOREPLIFETIME(AYGOPlayerState, bHasEnteredBattlePhase);
}

void AYGOPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AYGOPlayerState::OnRep_LifePoints()
{
	// 生命值改變時觸發事件
	if (OnLifePointsChanged.IsBound())
	{
		OnLifePointsChanged.Broadcast(LifePoints, LifePoints);
	}
}

void AYGOPlayerState::SetLifePoints(int32 NewLP)
{
	if (!HasAuthority())
	{
		return; // 只有 Server 可以修改
	}

	int32 OldLP = LifePoints;
	LifePoints = FMath::Max(0, NewLP);

	if (OnLifePointsChanged.IsBound())
	{
		OnLifePointsChanged.Broadcast(OldLP, LifePoints);
	}
}

void AYGOPlayerState::ReduceLifePoints(int32 Amount)
{
	SetLifePoints(LifePoints - Amount);
}

void AYGOPlayerState::RecoverLife(int32 Amount)
{
	SetLifePoints(LifePoints + Amount);
}

void AYGOPlayerState::LoadDeck(const TArray<int32>& CardCodes)
{
	if (!HasAuthority())
	{
		return;
	}

	MainDeck.Empty();
	ExtraDeck.Empty();

	for (int32 CardCode : CardCodes)
	{
		FYGOCardInstance NewCard;
		NewCard.CardData.CardCode = CardCode;
		NewCard.InstanceID = NextInstanceID++;
		NewCard.OwnerPlayerID = YGOPlayerID;
		NewCard.ControllerPlayerID = YGOPlayerID;
		NewCard.Location = EYGOLocation::Deck;
		NewCard.Sequence = MainDeck.Num();

		// TODO: 從 DataTable 載入完整卡片資料
		// 這裡需要查詢你的 ygo04_-_cards.csv DataTable

		// 判斷是否為額外卡組怪獸 (Fusion, Synchro, Xyz, Link)
		uint32 CardType = NewCard.CardData.Type;
		bool bIsExtraDeck = (CardType & YGOCardType::Fusion) ||
		                    (CardType & YGOCardType::Synchro) ||
		                    (CardType & YGOCardType::Xyz) ||
		                    (CardType & YGOCardType::Link);

		if (bIsExtraDeck)
		{
			NewCard.Location = EYGOLocation::ExtraDeck;
			NewCard.Sequence = ExtraDeck.Num();
			ExtraDeck.Add(NewCard);
		}
		else
		{
			MainDeck.Add(NewCard);
		}
	}

	// 洗牌
	ShuffleDeck();

	UpdateCardCounts();
}

void AYGOPlayerState::ShuffleDeck()
{
	if (!HasAuthority())
	{
		return;
	}

	// Fisher-Yates 洗牌演算法
	for (int32 i = MainDeck.Num() - 1; i > 0; --i)
	{
		int32 j = FMath::RandRange(0, i);
		MainDeck.Swap(i, j);
	}

	// 更新 Sequence
	for (int32 i = 0; i < MainDeck.Num(); ++i)
	{
		MainDeck[i].Sequence = i;
	}
}

void AYGOPlayerState::DrawCards(int32 Count)
{
	if (!HasAuthority())
	{
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		if (MainDeck.Num() == 0)
		{
			// 卡組沒牌了 - 敗北條件
			UE_LOG(LogTemp, Warning, TEXT("Player %d - Deck out!"), YGOPlayerID);
			break;
		}

		// 從卡組頂抽牌
		FYGOCardInstance DrawnCard = MainDeck[0];
		MainDeck.RemoveAt(0);

		// 加入手牌
		AddCardToHand(DrawnCard);

		// 觸發抽牌事件
		if (OnCardDrawn.IsBound())
		{
			OnCardDrawn.Broadcast(DrawnCard);
		}
	}

	UpdateCardCounts();

	// 發送手牌給客戶端 (僅擁有者)
	if (GetOwner())
	{
		APlayerController* PC = Cast<APlayerController>(GetOwner());
		if (PC)
		{
			Client_ReceiveHandCards(Hand);
		}
	}
}

void AYGOPlayerState::AddCardToHand(const FYGOCardInstance& Card)
{
	if (!HasAuthority())
	{
		return;
	}

	FYGOCardInstance NewCard = Card;
	NewCard.Location = EYGOLocation::Hand;
	NewCard.Sequence = Hand.Num();
	Hand.Add(NewCard);

	UpdateCardCounts();
}

void AYGOPlayerState::RemoveCardFromHand(int32 InstanceID)
{
	if (!HasAuthority())
	{
		return;
	}

	for (int32 i = 0; i < Hand.Num(); ++i)
	{
		if (Hand[i].InstanceID == InstanceID)
		{
			Hand.RemoveAt(i);
			UpdateCardCounts();
			return;
		}
	}
}

void AYGOPlayerState::SendToGraveyard(const FYGOCardInstance& Card)
{
	if (!HasAuthority())
	{
		return;
	}

	FYGOCardInstance GraveCard = Card;
	GraveCard.Location = EYGOLocation::Graveyard;
	GraveCard.Sequence = Graveyard.Num();
	Graveyard.Add(GraveCard);
}

void AYGOPlayerState::UpdateCardCounts()
{
	if (!HasAuthority())
	{
		return;
	}

	DeckCount = MainDeck.Num();
	ExtraDeckCount = ExtraDeck.Num();
	HandCount = Hand.Num();
}

void AYGOPlayerState::Client_ReceiveHandCards_Implementation(const TArray<FYGOCardInstance>& HandCards)
{
	// 客戶端接收手牌資訊
	ClientHand = HandCards;
}
