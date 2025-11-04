// YGO_unreal - Player Controller Implementation

#include "YGOPlayerController.h"
#include "YGOGameState.h"
#include "YGOPlayerState.h"
#include "YGOCardActor.h"
#include "YGOFieldZone.h"

AYGOPlayerController::AYGOPlayerController()
{
	MyPlayerID = 0;
	SelectedCard = nullptr;
	SelectedZone = nullptr;

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AYGOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 從 PlayerState 取得玩家 ID
	AYGOPlayerState* YGOPlayerState = GetPlayerState<AYGOPlayerState>();
	if (YGOPlayerState)
	{
		MyPlayerID = YGOPlayerState->YGOPlayerID;
	}
}

void AYGOPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 綁定輸入事件
	// 註: 這裡假設你使用傳統 InputComponent
	// 如果使用 Enhanced Input,需要改用 InputAction

	if (InputComponent)
	{
		// 滑鼠左鍵點擊
		InputComponent->BindAction("MouseClick", IE_Pressed, this, &AYGOPlayerController::OnMouseClick);

		// ESC 或右鍵取消
		InputComponent->BindAction("Cancel", IE_Pressed, this, &AYGOPlayerController::OnCancel);
	}
}

bool AYGOPlayerController::IsMyTurn() const
{
	AYGOGameState* GameState = GetWorld()->GetGameState<AYGOGameState>();
	if (GameState)
	{
		return GameState->CurrentPlayer == MyPlayerID;
	}
	return false;
}

void AYGOPlayerController::SelectCard(AYGOCardActor* Card)
{
	SelectedCard = Card;

	if (OnSelectionChanged.IsBound())
	{
		OnSelectionChanged.Broadcast(Card);
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerController] Selected card: %s"),
	       Card ? *Card->CardInstance.CardData.CardName : TEXT("None"));
}

void AYGOPlayerController::DeselectCard()
{
	SelectCard(nullptr);
}

void AYGOPlayerController::SelectZone(AYGOFieldZone* Zone)
{
	SelectedZone = Zone;
	UE_LOG(LogTemp, Log, TEXT("[PlayerController] Selected zone: Player %d, Sequence %d"),
	       Zone ? Zone->PlayerID : 255, Zone ? Zone->Sequence : 255);
}

void AYGOPlayerController::OnMouseClick()
{
	// 射線檢測
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		// 檢查是否點擊到卡片
		AYGOCardActor* ClickedCard = Cast<AYGOCardActor>(HitResult.GetActor());
		if (ClickedCard)
		{
			SelectCard(ClickedCard);
			return;
		}

		// 檢查是否點擊到場地位置
		AYGOFieldZone* ClickedZone = Cast<AYGOFieldZone>(HitResult.GetActor());
		if (ClickedZone)
		{
			SelectZone(ClickedZone);
			return;
		}
	}

	// 沒點到任何東西,取消選擇
	DeselectCard();
}

void AYGOPlayerController::OnCancel()
{
	DeselectCard();
}

void AYGOPlayerController::RequestNormalSummon(AYGOCardActor* Card, AYGOFieldZone* TargetZone)
{
	if (!Card || !TargetZone)
	{
		return;
	}

	Server_NormalSummon(Card, TargetZone);
}

void AYGOPlayerController::Server_NormalSummon_Implementation(AYGOCardActor* Card, AYGOFieldZone* TargetZone)
{
	// Server 驗證並執行召喚
	if (!Card || !TargetZone)
	{
		Client_ShowMessage(TEXT("Invalid summon target!"));
		return;
	}

	// 檢查是否輪到此玩家
	AYGOGameState* GameState = GetWorld()->GetGameState<AYGOGameState>();
	if (!GameState || GameState->CurrentPlayer != MyPlayerID)
	{
		Client_ShowMessage(TEXT("Not your turn!"));
		return;
	}

	// 檢查是否已經通常召喚過
	AYGOPlayerState* YGOPlayerState = GetPlayerState<AYGOPlayerState>();
	if (YGOPlayerState && YGOPlayerState->bHasNormalSummoned)
	{
		Client_ShowMessage(TEXT("Already normal summoned this turn!"));
		return;
	}

	// TODO: 更多驗證 (卡片是否在手牌、是否需要祭品等)

	// 執行召喚
	Card->CardInstance.Location = EYGOLocation::MonsterZone;
	Card->CardInstance.Sequence = TargetZone->Sequence;
	Card->CardInstance.Position = EYGOPosition::FaceUpAttack;
	Card->CardInstance.ControllerPlayerID = MyPlayerID;

	// 放置到場地
	TargetZone->PlaceCard(Card);
	GameState->PlaceCardOnField(Card, MyPlayerID, EYGOLocation::MonsterZone, TargetZone->Sequence);

	// 標記已召喚
	if (YGOPlayerState)
	{
		YGOPlayerState->bHasNormalSummoned = true;
		YGOPlayerState->NormalSummonCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("[Server] Normal summoned: %s"), *Card->CardInstance.CardData.CardName);
}

void AYGOPlayerController::RequestSetCard(AYGOCardActor* Card, AYGOFieldZone* TargetZone)
{
	if (!Card || !TargetZone)
	{
		return;
	}

	Server_SetCard(Card, TargetZone);
}

void AYGOPlayerController::Server_SetCard_Implementation(AYGOCardActor* Card, AYGOFieldZone* TargetZone)
{
	// Similar to normal summon but face-down
	if (!Card || !TargetZone)
	{
		return;
	}

	AYGOGameState* GameState = GetWorld()->GetGameState<AYGOGameState>();
	if (!GameState || GameState->CurrentPlayer != MyPlayerID)
	{
		return;
	}

	Card->CardInstance.Location = EYGOLocation::MonsterZone;
	Card->CardInstance.Sequence = TargetZone->Sequence;
	Card->CardInstance.Position = EYGOPosition::FaceDownDefense;

	TargetZone->PlaceCard(Card);
	GameState->PlaceCardOnField(Card, MyPlayerID, EYGOLocation::MonsterZone, TargetZone->Sequence);
}

void AYGOPlayerController::RequestAttack(AYGOCardActor* Attacker, AYGOCardActor* Target)
{
	if (!Attacker)
	{
		return;
	}

	Server_Attack(Attacker, Target);
}

void AYGOPlayerController::Server_Attack_Implementation(AYGOCardActor* Attacker, AYGOCardActor* Target)
{
	// TODO: 實作完整戰鬥邏輯
	if (!Attacker)
	{
		return;
	}

	if (Target)
	{
		Attacker->AttackCard(Target);
	}
	else
	{
		Attacker->DirectAttack();
	}
}

void AYGOPlayerController::RequestEndPhase()
{
	Server_EndPhase();
}

void AYGOPlayerController::Server_EndPhase_Implementation()
{
	AYGOGameState* GameState = GetWorld()->GetGameState<AYGOGameState>();
	if (GameState && GameState->CurrentPlayer == MyPlayerID)
	{
		GameState->GoToNextPhase();
	}
}

void AYGOPlayerController::RequestEndTurn()
{
	Server_EndTurn();
}

void AYGOPlayerController::Server_EndTurn_Implementation()
{
	AYGOGameState* GameState = GetWorld()->GetGameState<AYGOGameState>();
	if (GameState && GameState->CurrentPlayer == MyPlayerID)
	{
		GameState->GoToNextTurn();
	}
}

void AYGOPlayerController::Client_ShowMessage_Implementation(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("[Message] %s"), *Message);

	// TODO: 顯示 UI 訊息
	// 可以觸發 Widget 顯示訊息
}

void AYGOPlayerController::Client_RequestSelectCard_Implementation(int32 Min, int32 Max, const TArray<AYGOCardActor*>& SelectableCards)
{
	// TODO: 顯示卡片選擇 UI
	UE_LOG(LogTemp, Log, TEXT("[Client] Select %d to %d cards from %d options"), Min, Max, SelectableCards.Num());
}

void AYGOPlayerController::Client_RequestSelectPlace_Implementation(uint8 PlayerID, EYGOLocation Zone)
{
	// TODO: 顯示位置選擇 UI
	UE_LOG(LogTemp, Log, TEXT("[Client] Select place for Player %d in zone %d"), PlayerID, static_cast<uint8>(Zone));
}

void AYGOPlayerController::Client_RequestYesNo_Implementation(const FString& Question)
{
	// TODO: 顯示是/否對話框
	UE_LOG(LogTemp, Log, TEXT("[Client] Yes/No: %s"), *Question);
}

void AYGOPlayerController::Server_RespondSelectCard_Implementation(const TArray<AYGOCardActor*>& SelectedCards)
{
	// Server 接收玩家選擇的卡片
	UE_LOG(LogTemp, Log, TEXT("[Server] Player selected %d cards"), SelectedCards.Num());

	// TODO: 將選擇結果傳遞給 ygopro-core
}

void AYGOPlayerController::Server_RespondSelectPlace_Implementation(AYGOFieldZone* SelectedZoneParam)
{
	// Server 接收玩家選擇的位置
	if (SelectedZoneParam)
	{
		UE_LOG(LogTemp, Log, TEXT("[Server] Player selected zone: %d"), SelectedZoneParam->Sequence);
	}

	// TODO: 將選擇結果傳遞給 ygopro-core
}

void AYGOPlayerController::Server_RespondYesNo_Implementation(bool bYes)
{
	// Server 接收玩家的是/否決策
	UE_LOG(LogTemp, Log, TEXT("[Server] Player responded: %s"), bYes ? TEXT("Yes") : TEXT("No"));

	// TODO: 將選擇結果傳遞給 ygopro-core
}
