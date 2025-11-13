// YGO_unreal - Player State Implementation

#include "YGODataTableSubsystem.h"
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

	UDataTable *CardDataTable = GetGameInstance()
									->GetSubsystem<UYGODataTableSubsystem>()
									->GetDataTable(TEXT("ygo04 - cards"));
	if (!CardDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("CardDataTable is null"));
		return;
	}

	UDataTable *CardNameDataTable = GetGameInstance()
										->GetSubsystem<UYGODataTableSubsystem>()
										->GetDataTable(TEXT("ygo04_-_name"));
	if (!CardNameDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("CardNameDataTable is null"));
		return;
	}

	MainDeck.Empty();
	ExtraDeck.Empty();

	for (int32 CardCode : CardCodes)
	{
		// UE_LOG(LogTemp, Log, TEXT("[PlayerState] Loaded deck for card code %d"),
		//	CardCode);
		FName RowName = FName(*FString::FromInt(CardCode));
		const FYGOCardDataRow *CardDataRow =
			CardDataTable->FindRow<FYGOCardDataRow>(
				RowName, TEXT("CardDataLookup"));
		if (!CardDataRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("No CardDataRow found for CardCode: %d"), CardCode);
			continue;
		}

		const FYGOCardTextRow *CardTextRow = CardNameDataTable->FindRow<FYGOCardTextRow>(
			RowName, TEXT("CardNameLookup"));
		if (!CardTextRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("No CardTextRow found for CardCode: %d"), CardCode);
			continue;
		}

		FYGOCardInstance NewCard;
		NewCard.CardData.CardCode = CardCode;
		NewCard.CardData.CardName = CardTextRow->en;
		NewCard.CardData.Type = GetCardType(*CardDataRow);
		NewCard.CardData.Attribute = (EYGOAttribute)CardDataRow->attribute;
		NewCard.CardData.Race = (EYGORace)CardDataRow->monsterType;
		NewCard.CardData.Level = CardDataRow->level;
		NewCard.CardData.Attack = CardDataRow->attack;
		NewCard.CardData.Defense = CardDataRow->defense;

		NewCard.InstanceID = NextInstanceID++;
		NewCard.OwnerPlayerID = YGOPlayerID;
		NewCard.ControllerPlayerID = YGOPlayerID;
		NewCard.Location = EYGOLocation::Deck;
		NewCard.Sequence = MainDeck.Num();

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

	// 取得 GameState 中的牌組位置
	AYGOGameState *GameState = GetWorld()->GetGameState<AYGOGameState>();
	if (!GameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerState] GameState not found"));
		return;
	}

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

void AYGOPlayerState::SpawnAllDeckCards()
{
	// 清除舊的牌組卡片
	ClearDeckCardActors();

	// 取得 GameState 中的牌組位置
	AYGOGameState *GameState = GetWorld()->GetGameState<AYGOGameState>();
	if (!GameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerState] GameState not found"));
		return;
	}

	AYGOFieldZone *DeckZone = (YGOPlayerID == 0) ? GameState->Player0_DeckPosition : GameState->Player1_DeckPosition;
	if (!DeckZone)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerState] Deck position not found for Player %d"), YGOPlayerID);
		return;
	}

	// 取得牌組基礎位置和旋轉
	FVector DeckBaseLocation = DeckZone->GetActorLocation();
	FRotator DeckRotation = DeckZone->GetActorRotation();

	// 卡片堆疊參數
	const float CardThickness = 0.05f; // 每張卡的厚度（Z軸偏移）

	UE_LOG(LogTemp, Log, TEXT("[PlayerState] Spawning %d deck cards for Player %d"), MainDeck.Num(), YGOPlayerID);

	// 為主牌組的每張卡片生成 Actor
	for (int32 i = 0; i < MainDeck.Num(); ++i)
	{
		const FYGOCardInstance &CardInstance = MainDeck[i];

		// 生成參數
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 生成卡片 Actor
		AYGOCardActor *NewCardActor = GetWorld()->SpawnActor<AYGOCardActor>(
			CardActorClass,
			DeckBaseLocation,
			DeckRotation,
			SpawnParams);

		if (NewCardActor)
		{
			// 設定卡片資料（保持牌組資料不變）
			FYGOCardInstance DeckCardData = CardInstance;
			DeckCardData.Position = EYGOPosition::FaceDownDefense;
			NewCardActor->SetCardData(DeckCardData);
			NewCardActor->FlipFaceDown();
			NewCardActor->MoveToZone(DeckZone);

			// 禁用碰撞（牌組卡片不需要互動）
			if (NewCardActor->CardFrontMesh)
			{
				NewCardActor->CardFrontMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			if (NewCardActor->CardBackMesh)
			{
				NewCardActor->CardBackMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			// 加入陣列
			DeckCardActors.Add(NewCardActor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerState] Successfully spawned %d deck card actors for Player %d"),
		   DeckCardActors.Num(), YGOPlayerID);
}

void AYGOPlayerState::SpawnAllExtraDeckCards()
{
	// 清除舊的額外牌組卡片
	for (AYGOCardActor *CardActor : ExtraDeckCardActors)
	{
		if (CardActor)
		{
			CardActor->Destroy();
		}
	}
	ExtraDeckCardActors.Empty();

	// 如果額外牌組為空，直接返回
	if (ExtraDeck.Num() == 0)
	{
		return;
	}

	// 取得 GameState 中的額外牌組位置
	AYGOGameState *GameState = GetWorld()->GetGameState<AYGOGameState>();
	if (!GameState)
	{
		return;
	}

	AYGOFieldZone *ExtraDeckZone = (YGOPlayerID == 0) ? GameState->Player0_ExtraDeckPosition : GameState->Player1_ExtraDeckPosition;
	if (!ExtraDeckZone)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerState] Extra deck position not found for Player %d"), YGOPlayerID);
		return;
	}

	// 取得額外牌組基礎位置和旋轉
	FVector ExtraDeckBaseLocation = ExtraDeckZone->GetActorLocation();
	FRotator ExtraDeckRotation = ExtraDeckZone->GetActorRotation();

	// 卡片堆疊參數
	const float CardThickness = 0.05f;

	UE_LOG(LogTemp, Log, TEXT("[PlayerState] Spawning %d extra deck cards for Player %d"), ExtraDeck.Num(), YGOPlayerID);

	// 為額外牌組的每張卡片生成 Actor
	for (int32 i = 0; i < ExtraDeck.Num(); ++i)
	{
		const FYGOCardInstance &CardInstance = ExtraDeck[i];

		// 計算堆疊位置
		FVector CardLocation = ExtraDeckBaseLocation + FVector(0, 0, i * CardThickness);

		// 生成參數
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 生成卡片 Actor
		AYGOCardActor *NewCardActor = GetWorld()->SpawnActor<AYGOCardActor>(
			CardActorClass,
			CardLocation,
			ExtraDeckRotation,
			SpawnParams);

		if (NewCardActor)
		{
			// 設定卡片資料
			FYGOCardInstance ExtraCardData = CardInstance;
			ExtraCardData.Position = EYGOPosition::FaceDownDefense;
			NewCardActor->SetCardData(ExtraCardData);
			NewCardActor->FlipFaceDown();

			// 禁用碰撞
			if (NewCardActor->CardFrontMesh)
			{
				NewCardActor->CardFrontMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			if (NewCardActor->CardBackMesh)
			{
				NewCardActor->CardBackMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			// 加入陣列
			ExtraDeckCardActors.Add(NewCardActor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerState] Successfully spawned %d extra deck card actors for Player %d"),
		   ExtraDeckCardActors.Num(), YGOPlayerID);
}

void AYGOPlayerState::ClearDeckCardActors()
{
	// 清除主牌組卡片
	for (AYGOCardActor *CardActor : DeckCardActors)
	{
		if (CardActor)
		{
			CardActor->Destroy();
		}
	}
	DeckCardActors.Empty();

	// 清除額外牌組卡片
	for (AYGOCardActor *CardActor : ExtraDeckCardActors)
	{
		if (CardActor)
		{
			CardActor->Destroy();
		}
	}
	ExtraDeckCardActors.Empty();

	UE_LOG(LogTemp, Log, TEXT("[PlayerState] Cleared all deck card actors for Player %d"), YGOPlayerID);
}

uint32 AYGOPlayerState::GetCardType(const FYGOCardDataRow &cardDataRow)
{
	uint32 result = 0;
	switch (cardDataRow.cardCategory)
	{
	case 0:
		result |= YGOCardType::Monster;
		break;
	case 1:
		result |= YGOCardType::Spell;
		break;
	case 2:
		result |= YGOCardType::Trap;
		break;
	default:
		break;
	}

	// =============================
	// 怪獸卡
	// =============================
	if (cardDataRow.cardCategory == 0)
	{
		// Class
		switch ((EYGOMonsterClass)cardDataRow.monsterClass)
		{
		case EYGOMonsterClass::Normal:
			result |= YGOCardType::Normal;
			break;
		case EYGOMonsterClass::Effect:
			result |= YGOCardType::Effect;
			break;
		case EYGOMonsterClass::Fusion:
			result |= YGOCardType::Fusion;
			break;
		case EYGOMonsterClass::Ritual:
			result |= YGOCardType::Ritual;
			break;
		case EYGOMonsterClass::TrapMonster:
			result |= YGOCardType::TrapMonster;
			break;
		case EYGOMonsterClass::Synchro:
			result |= YGOCardType::Synchro;
			break;
		case EYGOMonsterClass::Token:
			result |= YGOCardType::Token;
			break;
		case EYGOMonsterClass::Xyz:
			result |= YGOCardType::Xyz;
			break;
		case EYGOMonsterClass::Pendulum:
			result |= YGOCardType::Pendulum;
			break;
		case EYGOMonsterClass::Link:
			result |= YGOCardType::Link;
			break;
		default:
			break;
		}

		// Trait
		switch ((EYGOMonsterTrait)cardDataRow.monsterTrait)
		{
		case EYGOMonsterTrait::Spirit:
			result |= YGOCardType::Spirit;
			break;
		case EYGOMonsterTrait::Union:
			result |= YGOCardType::Union;
			break;
		case EYGOMonsterTrait::Gemini:
			result |= YGOCardType::Gemini;
			break;
		case EYGOMonsterTrait::Tuner:
			result |= YGOCardType::Tuner;
			break;
		case EYGOMonsterTrait::Flip:
			result |= YGOCardType::Flip;
			break;
		case EYGOMonsterTrait::Toon:
			result |= YGOCardType::Toon;
			break;
		case EYGOMonsterTrait::SpecialSummon:
			result |= YGOCardType::SpecialSummon;
			break;
		default:
			break;
		}
	}
	// =============================
	// 魔法／陷阱 Icon 類型
	// =============================
	else if (cardDataRow.cardCategory == 1 || cardDataRow.cardCategory == 2)
	{
		switch ((EYGOIcon)cardDataRow.icon)
		{
		case EYGOIcon::Normal:
			result |= YGOCardType::Normal;
			break;
		case EYGOIcon::Equip:
			result |= YGOCardType::Equip;
			break;
		case EYGOIcon::Field:
			result |= YGOCardType::Field;
			break;
		case EYGOIcon::QuickPlay:
			result |= YGOCardType::QuickPlay;
			break;
		case EYGOIcon::Ritual:
			result |= YGOCardType::Ritual;
			break;
		case EYGOIcon::Continuous:
			result |= YGOCardType::Continuous;
			break;
		case EYGOIcon::Counter:
			result |= YGOCardType::Counter;
			break;
		default:
			break;
		}
	}

	return result;
}
