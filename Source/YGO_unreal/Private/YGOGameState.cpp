// YGO_unreal - Game State Implementation

#include "YGOFieldZone.h"
#include "YGOGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AYGOGameState::AYGOGameState()
{
	TurnCount = 0;
	CurrentPlayer = 0;
	CurrentPhase = EYGOPhase::None;
	bGameStarted = false;
	bGameEnded = false;
	WinnerPlayerID = 255; // 無效值

	bReplicates = true;
}

void AYGOGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYGOGameState, TurnCount);
	DOREPLIFETIME(AYGOGameState, CurrentPlayer);
	DOREPLIFETIME(AYGOGameState, CurrentPhase);
	DOREPLIFETIME(AYGOGameState, Player0_MonsterZone);
	DOREPLIFETIME(AYGOGameState, Player0_SpellTrapZone);
	DOREPLIFETIME(AYGOGameState, Player1_MonsterZone);
	DOREPLIFETIME(AYGOGameState, Player1_SpellTrapZone);
	DOREPLIFETIME(AYGOGameState, FieldSpellZone);
	DOREPLIFETIME(AYGOGameState, bGameStarted);
	DOREPLIFETIME(AYGOGameState, bGameEnded);
	DOREPLIFETIME(AYGOGameState, WinnerPlayerID);
}

void AYGOGameState::BeginPlay()
{
	Super::BeginPlay();

	InitializeFieldZones();
}

void AYGOGameState::InitializeFieldZones()
{
	// 初始化場地區域為 5 格 (遊戲王標準)
	Player0_MonsterZone.SetNum(5);
	Player0_SpellTrapZone.SetNum(5);
	Player1_MonsterZone.SetNum(5);
	Player1_SpellTrapZone.SetNum(5);
	FieldSpellZone.SetNum(2); // 雙方各一個場地魔法區

	// 初始化為 nullptr
	for (int32 i = 0; i < 5; ++i)
	{
		Player0_MonsterZone[i] = nullptr;
		Player0_SpellTrapZone[i] = nullptr;
		Player1_MonsterZone[i] = nullptr;
		Player1_SpellTrapZone[i] = nullptr;
	}

	FieldSpellZone[0] = nullptr;
	FieldSpellZone[1] = nullptr;

	// 初始化牌組位置為 nullptr
	Player0_DeckPosition = nullptr;
	Player1_DeckPosition = nullptr;

	if (HasAuthority())
	{
		TArray<AActor *> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AYGOFieldZone::StaticClass(), FoundActors);
		int32 count = 0;
		for (AActor *Actor : FoundActors)
		{

			if (AYGOFieldZone *Zone = Cast<AYGOFieldZone>(Actor))
			{
				RegisterFieldZone(Zone);
				count++;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[GameState] Collected %d FieldZones."), count);
	}
}

void AYGOGameState::RegisterFieldZone(AYGOFieldZone *Zone)
{
	if (!Zone || !HasAuthority())
	{
		return;
	}

	// UE_LOG(LogTemp, Log, TEXT("[GameState] Registering FieldZone: Player %d, Type %d, Seq %d"),
	// 	   Zone->PlayerID, static_cast<uint8>(Zone->ZoneType), Zone->Sequence);

	// 根據 PlayerID 和 ZoneType 註冊到對應陣列
	if (Zone->PlayerID == 0)
	{
		if (Zone->ZoneType == EYGOLocation::MonsterZone && Player0_MonsterZonePositions.Num() < 5)
		{
			Player0_MonsterZonePositions.Add(Zone);
		}
		else if (Zone->ZoneType == EYGOLocation::SpellTrapZone && Player0_SpellTrapZonePositions.Num() < 5)
		{
			Player0_SpellTrapZonePositions.Add(Zone);
		}
		else if (Zone->ZoneType == EYGOLocation::Deck)
		{
			Player0_DeckPosition = Zone;
			UE_LOG(LogTemp, Log, TEXT("[GameState] Registered Player 0 Deck position"));
		}
	}
	else if (Zone->PlayerID == 1)
	{
		if (Zone->ZoneType == EYGOLocation::MonsterZone && Player1_MonsterZonePositions.Num() < 5)
		{
			Player1_MonsterZonePositions.Add(Zone);
		}
		else if (Zone->ZoneType == EYGOLocation::SpellTrapZone && Player1_SpellTrapZonePositions.Num() < 5)
		{
			Player1_SpellTrapZonePositions.Add(Zone);
		}
		else if (Zone->ZoneType == EYGOLocation::Deck)
		{
			Player1_DeckPosition = Zone;
			UE_LOG(LogTemp, Log, TEXT("[GameState] Registered Player 1 Deck position"));
		}
	}
}

void AYGOGameState::OnRep_TurnCount()
{
	if (OnTurnChanged.IsBound())
	{
		OnTurnChanged.Broadcast(TurnCount, CurrentPlayer);
	}
}

void AYGOGameState::OnRep_CurrentPlayer()
{
	if (OnTurnChanged.IsBound())
	{
		OnTurnChanged.Broadcast(TurnCount, CurrentPlayer);
	}
}

void AYGOGameState::OnRep_CurrentPhase()
{
	if (OnPhaseChanged.IsBound())
	{
		OnPhaseChanged.Broadcast(CurrentPhase);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameState] Phase changed to: %d"), static_cast<uint8>(CurrentPhase));
}

void AYGOGameState::GoToNextTurn()
{
	if (!HasAuthority())
	{
		return;
	}

	// 切換玩家
	CurrentPlayer = (CurrentPlayer == 0) ? 1 : 0;
	TurnCount++;

	// 重置到抽牌階段
	SetPhase(EYGOPhase::Draw);

	if (OnTurnChanged.IsBound())
	{
		OnTurnChanged.Broadcast(TurnCount, CurrentPlayer);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameState] Turn %d - Player %d"), TurnCount, CurrentPlayer);
}

void AYGOGameState::GoToNextPhase()
{
	if (!HasAuthority())
	{
		return;
	}

	// 階段順序: Draw -> Standby -> Main1 -> BattleStart -> Battle -> Main2 -> End
	switch (CurrentPhase)
	{
	case EYGOPhase::Draw:
		SetPhase(EYGOPhase::Standby);
		break;
	case EYGOPhase::Standby:
		SetPhase(EYGOPhase::Main1);
		break;
	case EYGOPhase::Main1:
		SetPhase(EYGOPhase::BattleStart);
		break;
	case EYGOPhase::BattleStart:
		SetPhase(EYGOPhase::Battle);
		break;
	case EYGOPhase::Battle:
		SetPhase(EYGOPhase::Main2);
		break;
	case EYGOPhase::Main2:
		SetPhase(EYGOPhase::End);
		break;
	case EYGOPhase::End:
		GoToNextTurn();
		break;
	default:
		SetPhase(EYGOPhase::Draw);
		break;
	}
}

void AYGOGameState::SetPhase(EYGOPhase NewPhase)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentPhase = NewPhase;

	if (OnPhaseChanged.IsBound())
	{
		OnPhaseChanged.Broadcast(CurrentPhase);
	}
}

TArray<AYGOCardActor *> *AYGOGameState::GetZoneArray(uint8 PlayerID, EYGOLocation Zone)
{
	if (PlayerID == 0)
	{
		if (Zone == EYGOLocation::MonsterZone)
		{
			return &Player0_MonsterZone;
		}
		else if (Zone == EYGOLocation::SpellTrapZone)
		{
			return &Player0_SpellTrapZone;
		}
	}
	else if (PlayerID == 1)
	{
		if (Zone == EYGOLocation::MonsterZone)
		{
			return &Player1_MonsterZone;
		}
		else if (Zone == EYGOLocation::SpellTrapZone)
		{
			return &Player1_SpellTrapZone;
		}
	}

	if (Zone == EYGOLocation::FieldZone)
	{
		return &FieldSpellZone;
	}

	return nullptr;
}

bool AYGOGameState::PlaceCardOnField(AYGOCardActor *Card, uint8 PlayerID, EYGOLocation Zone, uint8 Sequence)
{
	if (!HasAuthority() || !Card)
	{
		return false;
	}

	TArray<AYGOCardActor *> *ZoneArray = GetZoneArray(PlayerID, Zone);
	if (!ZoneArray || !ZoneArray->IsValidIndex(Sequence))
	{
		return false;
	}

	// 檢查位置是否已被佔用
	if ((*ZoneArray)[Sequence] != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameState] Zone already occupied!"));
		return false;
	}

	(*ZoneArray)[Sequence] = Card;
	return true;
}

bool AYGOGameState::RemoveCardFromField(AYGOCardActor *Card)
{
	if (!HasAuthority() || !Card)
	{
		return false;
	}

	// 搜尋並移除卡片
	for (int32 i = 0; i < Player0_MonsterZone.Num(); ++i)
	{
		if (Player0_MonsterZone[i] == Card)
		{
			Player0_MonsterZone[i] = nullptr;
			return true;
		}
	}

	for (int32 i = 0; i < Player0_SpellTrapZone.Num(); ++i)
	{
		if (Player0_SpellTrapZone[i] == Card)
		{
			Player0_SpellTrapZone[i] = nullptr;
			return true;
		}
	}

	for (int32 i = 0; i < Player1_MonsterZone.Num(); ++i)
	{
		if (Player1_MonsterZone[i] == Card)
		{
			Player1_MonsterZone[i] = nullptr;
			return true;
		}
	}

	for (int32 i = 0; i < Player1_SpellTrapZone.Num(); ++i)
	{
		if (Player1_SpellTrapZone[i] == Card)
		{
			Player1_SpellTrapZone[i] = nullptr;
			return true;
		}
	}

	return false;
}

AYGOCardActor *AYGOGameState::GetCardAt(uint8 PlayerID, EYGOLocation Zone, uint8 Sequence) const
{
	const TArray<AYGOCardActor *> *ZoneArray = const_cast<AYGOGameState *>(this)->GetZoneArray(PlayerID, Zone);
	if (!ZoneArray || !ZoneArray->IsValidIndex(Sequence))
	{
		return nullptr;
	}

	return (*ZoneArray)[Sequence];
}

bool AYGOGameState::IsZoneAvailable(uint8 PlayerID, EYGOLocation Zone, uint8 Sequence) const
{
	return GetCardAt(PlayerID, Zone, Sequence) == nullptr;
}

void AYGOGameState::StartGame()
{
	if (!HasAuthority())
	{
		return;
	}

	bGameStarted = true;
	TurnCount = 1;
	CurrentPlayer = 0; // 玩家 0 先攻
	SetPhase(EYGOPhase::Draw);

	if (OnGameStarted.IsBound())
	{
		OnGameStarted.Broadcast();
	}

	UE_LOG(LogTemp, Log, TEXT("[GameState] Game Started!"));
}

void AYGOGameState::EndGame(uint8 WinnerID)
{
	if (!HasAuthority())
	{
		return;
	}

	bGameEnded = true;
	WinnerPlayerID = WinnerID;

	if (OnGameEnded.IsBound())
	{
		OnGameEnded.Broadcast(WinnerID);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameState] Game Ended! Winner: Player %d"), WinnerID);
}
