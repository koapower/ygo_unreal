// YGO_unreal - Game Mode Implementation

#include "YGOGameMode.h"
#include "YGOGameState.h"
#include "YGOPlayerState.h"
#include "YGOPlayerController.h"

AYGOGameMode::AYGOGameMode()
{
	// 設定使用我們的自訂類別
	GameStateClass = AYGOGameState::StaticClass();
	PlayerStateClass = AYGOPlayerState::StaticClass();
	PlayerControllerClass = AYGOPlayerController::StaticClass();

	// 遊戲規則
	StartingLifePoints = 8000;
	StartingHandSize = 5;
	bFirstPlayerDraws = false; // 先攻第一回合不抽牌 (OCG 規則)

	ConnectedPlayers = 0;
	Player0 = nullptr;
	Player1 = nullptr;
}

void AYGOGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[GameMode] YGO Game Mode started!"));
}

void AYGOGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AYGOPlayerController* YGOPlayer = Cast<AYGOPlayerController>(NewPlayer);
	if (YGOPlayer)
	{
		AssignPlayerID(YGOPlayer);
		ConnectedPlayers++;

		UE_LOG(LogTemp, Log, TEXT("[GameMode] Player %d connected (%d/2)"),
		       YGOPlayer->MyPlayerID, ConnectedPlayers);

		// 當兩位玩家都連線時,初始化決鬥
		if (AreAllPlayersReady())
		{
			if (OnAllPlayersReady.IsBound())
			{
				OnAllPlayersReady.Broadcast();
			}

			// 延遲1秒後開始,讓 Client 完全載入
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AYGOGameMode::InitializeDuel, 1.0f, false);
		}
	}
}

void AYGOGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ConnectedPlayers--;
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Player disconnected (%d/2)"), ConnectedPlayers);

	// 如果有玩家離線,結束遊戲
	if (ConnectedPlayers < 2)
	{
		AYGOGameState* YGOGameState = GetGameState<AYGOGameState>();
		if (YGOGameState && YGOGameState->bGameStarted && !YGOGameState->bGameEnded)
		{
			EndDuel(255, TEXT("Player disconnected"));
		}
	}
}

void AYGOGameMode::AssignPlayerID(AYGOPlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	// 分配玩家 ID
	if (!Player0)
	{
		Player0 = PlayerController;
		PlayerController->MyPlayerID = 0;

		// 設定 PlayerState
		AYGOPlayerState* PlayerState = PlayerController->GetPlayerState<AYGOPlayerState>();
		if (PlayerState)
		{
			PlayerState->YGOPlayerID = 0;
		}
	}
	else if (!Player1)
	{
		Player1 = PlayerController;
		PlayerController->MyPlayerID = 1;

		// 設定 PlayerState
		AYGOPlayerState* PlayerState = PlayerController->GetPlayerState<AYGOPlayerState>();
		if (PlayerState)
		{
			PlayerState->YGOPlayerID = 1;
		}
	}
}

bool AYGOGameMode::AreAllPlayersReady() const
{
	return Player0 != nullptr && Player1 != nullptr && ConnectedPlayers == 2;
}

void AYGOGameMode::InitializeDuel()
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Initializing duel..."));

	if (!AreAllPlayersReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Cannot start duel - not all players ready"));
		return;
	}

	// 設定生命值
	AYGOPlayerState* PlayerState0 = Player0->GetPlayerState<AYGOPlayerState>();
	AYGOPlayerState* PlayerState1 = Player1->GetPlayerState<AYGOPlayerState>();

	if (PlayerState0)
	{
		PlayerState0->SetLifePoints(StartingLifePoints);
	}

	if (PlayerState1)
	{
		PlayerState1->SetLifePoints(StartingLifePoints);
	}

	// 載入卡組
	LoadDefaultDeck(0);
	LoadDefaultDeck(1);

	// 洗牌
	if (PlayerState0)
	{
		PlayerState0->ShuffleDeck();
	}

	if (PlayerState1)
	{
		PlayerState1->ShuffleDeck();
	}

	// 抽起手牌
	if (PlayerState0)
	{
		PlayerState0->DrawCards(StartingHandSize);
	}

	if (PlayerState1)
	{
		PlayerState1->DrawCards(StartingHandSize);
	}

	// 生成牌組視覺化（延遲確保 FieldZone 已註冊）
	FTimerHandle DeckSpawnTimer;
	GetWorld()->GetTimerManager().SetTimer(DeckSpawnTimer, [PlayerState0, PlayerState1]()
	{
		if (PlayerState0)
		{
			PlayerState0->SpawnDeckCard();
		}
		if (PlayerState1)
		{
			PlayerState1->SpawnDeckCard();
		}
	}, 0.2f, false);

	// 延遲0.5秒後開始決鬥
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AYGOGameMode::StartDuel, 0.5f, false);
}

void AYGOGameMode::StartDuel()
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Starting duel!"));

	AYGOGameState* YGOGameState = GetGameState<AYGOGameState>();
	if (YGOGameState)
	{
		YGOGameState->StartGame();

		// 綁定階段改變事件
		YGOGameState->OnPhaseChanged.AddDynamic(this, &AYGOGameMode::EnterPhase);

		// 開始第一回合
		StartNewTurn();
	}

	if (OnDuelStarted.IsBound())
	{
		OnDuelStarted.Broadcast();
	}
}

void AYGOGameMode::EndDuel(uint8 WinnerPlayerID, const FString& Reason)
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Duel ended! Winner: Player %d, Reason: %s"),
	       WinnerPlayerID, *Reason);

	AYGOGameState* YGOGameState = GetGameState<AYGOGameState>();
	if (YGOGameState)
	{
		YGOGameState->EndGame(WinnerPlayerID);
	}

	if (OnDuelEnded.IsBound())
	{
		OnDuelEnded.Broadcast(WinnerPlayerID, Reason);
	}
}

void AYGOGameMode::StartNewTurn()
{
	AYGOGameState* YGOGameState = GetGameState<AYGOGameState>();
	if (!YGOGameState)
	{
		return;
	}

	uint8 CurrentPlayer = YGOGameState->CurrentPlayer;
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Turn %d - Player %d"),
	       YGOGameState->TurnCount, CurrentPlayer);

	// 重置玩家回合狀態
	AYGOPlayerController* CurrentPlayerController = (CurrentPlayer == 0) ? Player0 : Player1;
	if (CurrentPlayerController)
	{
		AYGOPlayerState* PlayerState = CurrentPlayerController->GetPlayerState<AYGOPlayerState>();
		if (PlayerState)
		{
			PlayerState->bHasNormalSummoned = false;
			PlayerState->bHasEnteredBattlePhase = false;
		}
	}

	// 開始抽牌階段
	YGOGameState->SetPhase(EYGOPhase::Draw);
}

void AYGOGameMode::EnterPhase(EYGOPhase Phase)
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Entering phase: %d"), static_cast<uint8>(Phase));

	switch (Phase)
	{
	case EYGOPhase::Draw:
		ProcessDrawPhase();
		break;

	case EYGOPhase::Standby:
		ProcessStandbyPhase();
		break;

	// TODO: 其他階段的自動處理

	default:
		break;
	}
}

void AYGOGameMode::ProcessDrawPhase()
{
	AYGOGameState* YGOGameState = GetGameState<AYGOGameState>();
	if (!YGOGameState)
	{
		return;
	}

	uint8 CurrentPlayer = YGOGameState->CurrentPlayer;
	AYGOPlayerController* CurrentPlayerController = (CurrentPlayer == 0) ? Player0 : Player1;

	if (CurrentPlayerController)
	{
		AYGOPlayerState* PlayerState = CurrentPlayerController->GetPlayerState<AYGOPlayerState>();
		if (PlayerState)
		{
			// 第一回合先攻不抽牌 (OCG 規則)
			if (YGOGameState->TurnCount == 1 && CurrentPlayer == 0 && !bFirstPlayerDraws)
			{
				UE_LOG(LogTemp, Log, TEXT("[GameMode] First turn - skip draw"));
			}
			else
			{
				PlayerState->DrawCards(1);
				UE_LOG(LogTemp, Log, TEXT("[GameMode] Player %d draws a card"), CurrentPlayer);
			}
		}
	}

	// 自動進入待機階段
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, YGOGameState]()
	{
		if (YGOGameState)
		{
			YGOGameState->SetPhase(EYGOPhase::Standby);
		}
	}, 0.5f, false);
}

void AYGOGameMode::ProcessStandbyPhase()
{
	// 待機階段通常不需要自動處理,等待玩家操作
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Standby phase - waiting for player action"));
}

void AYGOGameMode::LoadDefaultDeck(uint8 PlayerID)
{
	// 載入預設測試卡組 (40張怪獸卡)
	TArray<int32> DefaultDeck;

	// TODO: 這裡應該從你的 CSV DataTable 載入實際的卡片代碼
	// 目前用假的代碼代替
	for (int32 i = 0; i < 40; ++i)
	{
		DefaultDeck.Add(1000 + i); // 假的卡片代碼
	}

	LoadDeckFromArray(PlayerID, DefaultDeck);
}

void AYGOGameMode::LoadDeckFromArray(uint8 PlayerID, const TArray<int32>& CardCodes)
{
	AYGOPlayerController* PlayerController = (PlayerID == 0) ? Player0 : Player1;
	if (PlayerController)
	{
		AYGOPlayerState* PlayerState = PlayerController->GetPlayerState<AYGOPlayerState>();
		if (PlayerState)
		{
			PlayerState->LoadDeck(CardCodes);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] Loaded deck for Player %d: %d cards"),
			       PlayerID, CardCodes.Num());
		}
	}
}
