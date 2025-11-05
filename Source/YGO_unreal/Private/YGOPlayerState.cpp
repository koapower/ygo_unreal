// YGO_unreal - Player State Implementation

#include "YGOFieldZone.h"
#include "YGOPlayerState.h"
#include "YGOCardActor.h"
#include "YGOGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

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

void AYGOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
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

void AYGOPlayerState::LoadDeck(const TArray<int32> &CardCodes)
{
	if (!HasAuthority())
	{
		return;
	}

	MainDeck.Empty();
	ExtraDeck.Empty();

	for (int32 CardCode : CardCodes)
	{
		//UE_LOG(LogTemp, Log, TEXT("[PlayerState] Loaded deck for card code %d"),
		//	CardCode);
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
		APlayerController *PC = Cast<APlayerController>(GetOwner());
		if (PC)
		{
			Client_ReceiveHandCards(Hand);
		}
	}
}

void AYGOPlayerState::AddCardToHand(const FYGOCardInstance &Card)
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

void AYGOPlayerState::SendToGraveyard(const FYGOCardInstance &Card)
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

void AYGOPlayerState::Client_ReceiveHandCards_Implementation(const TArray<FYGOCardInstance> &HandCards)
{
	// 客戶端接收手牌資訊
	ClientHand = HandCards;

	// 更新手牌視覺化
	SpawnHandCards();
}

// ========================================================================
// 卡片視覺化實作
// ========================================================================

void AYGOPlayerState::SpawnHandCards()
{
	// 清除舊的手牌 Actor
	for (AYGOCardActor *CardActor : HandCardActors)
	{
		if (CardActor)
		{
			CardActor->Destroy();
		}
	}
	HandCardActors.Empty();

	// 為每張手牌生成 Actor
	for (const FYGOCardInstance &CardInstance : ClientHand)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AYGOCardActor *NewCardActor = GetWorld()->SpawnActor<AYGOCardActor>(
			CardActorClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);

		if (NewCardActor)
		{
			NewCardActor->SetCardData(CardInstance);
			HandCardActors.Add(NewCardActor);
		}
	}

	// 更新手牌位置
	UpdateHandPositions();
}

void AYGOPlayerState::SpawnDeckCard()
{
	// 如果已有牌組 Actor，先銷毀
	if (DeckCardActor)
	{
		DeckCardActor->Destroy();
		DeckCardActor = nullptr;
	}

	// 如果牌組沒牌了，不生成
	if (DeckCount == 0)
	{
		return;
	}

	// 取得 GameState 中的牌組位置
	AYGOGameState *GameState = GetWorld()->GetGameState<AYGOGameState>();
	if (!GameState)
	{
		return;
	}

	AYGOFieldZone *DeckZone = (YGOPlayerID == 0) ? GameState->Player0_DeckPosition : GameState->Player1_DeckPosition;
	if (!DeckZone)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerState] Deck position not found for Player %d"), YGOPlayerID);
		return;
	}

	// 生成牌組卡片 Actor（背面朝上）
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	DeckCardActor = GetWorld()->SpawnActor<AYGOCardActor>(
		CardActorClass,
		DeckZone->GetActorLocation() + FVector(0, 0, 10),
		DeckZone->GetActorRotation(),
		SpawnParams);

	if (DeckCardActor)
	{
		// 設定為背面卡片
		FYGOCardInstance DeckCardData;
		DeckCardData.OwnerPlayerID = YGOPlayerID;
		DeckCardData.Location = EYGOLocation::Deck;
		DeckCardData.Position = EYGOPosition::FaceDownDefense;
		DeckCardActor->SetCardData(DeckCardData);
		DeckCardActor->FlipFaceDown();

		UE_LOG(LogTemp, Log, TEXT("[PlayerState] Spawned deck card for Player %d"), YGOPlayerID);
	}
}

void AYGOPlayerState::UpdateHandPositions()
{
	int32 HandSize = HandCardActors.Num();
	if (HandSize == 0)
	{
		return;
	}

	// 手牌排列參數
	const float CardSpacing = 80.0f; // 卡片間距
	const float ArcRadius = 1500.0f; // 弧形半徑
	const float HandHeight = 50.0f;	 // 手牌高度

	// 計算起始位置（以玩家為中心）
	float TotalWidth = (HandSize - 1) * CardSpacing;
	float StartX = -TotalWidth / 2.0f;

	// 根據玩家 ID 決定手牌位置（玩家 0 在下方，玩家 1 在上方）
	FVector BasePosition;
	FRotator BaseRotation;

	if (YGOPlayerID == 0)
	{
		// 玩家 0 - 手牌在螢幕下方
		BasePosition = FVector(0, 0, HandHeight);
		BaseRotation = FRotator(-90, 0, 0); // 正面朝上
	}
	else
	{
		// 玩家 1 - 手牌在螢幕上方
		BasePosition = FVector(0, 1000, HandHeight);
		BaseRotation = FRotator(-90, 180, 0); // 正面朝上但方向相反
	}

	// 排列手牌
	for (int32 i = 0; i < HandSize; ++i)
	{
		if (!HandCardActors[i])
		{
			continue;
		}

		// 計算 X 位置
		float XPos = StartX + (i * CardSpacing);

		// 計算弧形 Y 偏移
		float NormalizedX = (float)i / (float)(HandSize - 1); // 0 到 1
		if (HandSize == 1)
		{
			NormalizedX = 0.5f;
		}
		float Angle = (NormalizedX - 0.5f) * 30.0f; // -15 到 +15 度
		float YOffset = ArcRadius * FMath::Sin(FMath::DegreesToRadians(Angle));

		// 設定位置
		FVector CardPosition = BasePosition + FVector(XPos, YOffset, i * 0.5f); // 輕微 Z 偏移避免重疊
		HandCardActors[i]->SetActorLocation(CardPosition);

		// 設定旋轉（加上弧形角度）
		FRotator CardRotation = BaseRotation + FRotator(0, Angle, 0);
		HandCardActors[i]->SetActorRotation(CardRotation);
	}
}

void AYGOPlayerState::RemoveHandCardActor(AYGOCardActor *CardActor)
{
	if (CardActor)
	{
		HandCardActors.Remove(CardActor);
		CardActor->Destroy();

		// 重新排列剩餘手牌
		UpdateHandPositions();
	}
}
