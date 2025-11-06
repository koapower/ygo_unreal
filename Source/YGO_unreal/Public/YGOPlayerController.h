// YGO_unreal - Player Controller
// 處理玩家輸入和決策

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "YGOCoreTypes.h"
#include "YGOPlayerController.generated.h"

class AYGOCardActor;
class AYGOFieldZone;

/**
 * YGO 玩家控制器
 * 處理玩家的輸入和 UI 互動
 */
UCLASS()
class YGO_UNREAL_API AYGOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AYGOPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** 此物件是否已經初始化 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Initialization")
	bool bIsInitialized;

	/** 此物件已經load完場景，在beginplay時=true */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Initialization")
	bool bSelfIsReady;

	/** 因為本地以及同步過來的資料是非同步，所以要確保所有已須資訊備齊後才開始Init */
	UFUNCTION(BlueprintCallable, Category = "YGO|Initialization")
	void TryInitialize();

	// ========================================================================
	// 玩家 ID
	// ========================================================================

	/** 此玩家的 ID (0 或 1) */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Player")
	uint8 MyPlayerID;

	/** 取得玩家 ID */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Player")
	uint8 GetPlayerID() const { return MyPlayerID; }

	/** 是否輪到我的回合 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Player")
	bool IsMyTurn() const;

	// ========================================================================
	// 卡片選擇
	// ========================================================================

	/** 當前選中的卡片 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Selection")
	AYGOCardActor *SelectedCard;

	/** 選擇一張卡片 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Selection")
	void SelectCard(AYGOCardActor *Card);

	/** 取消選擇 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Selection")
	void DeselectCard();

	/** 當前選中的場地位置 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Selection")
	AYGOFieldZone *SelectedZone;

	/** 選擇場地位置 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Selection")
	void SelectZone(AYGOFieldZone *Zone);

	// ========================================================================
	// 遊戲操作 (Client → Server RPC)
	// ========================================================================

	/** 請求通常召喚 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Actions")
	void RequestNormalSummon(AYGOCardActor *Card, AYGOFieldZone *TargetZone);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "YGO|Actions")
	void Server_NormalSummon(AYGOCardActor *Card, AYGOFieldZone *TargetZone);

	/** 請求蓋放卡片 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Actions")
	void RequestSetCard(AYGOCardActor *Card, AYGOFieldZone *TargetZone);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "YGO|Actions")
	void Server_SetCard(AYGOCardActor *Card, AYGOFieldZone *TargetZone);

	/** 請求攻擊 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Actions")
	void RequestAttack(AYGOCardActor *Attacker, AYGOCardActor *Target);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "YGO|Actions")
	void Server_Attack(AYGOCardActor *Attacker, AYGOCardActor *Target);

	/** 請求結束階段 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Actions")
	void RequestEndPhase();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "YGO|Actions")
	void Server_EndPhase();

	/** 請求結束回合 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Actions")
	void RequestEndTurn();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "YGO|Actions")
	void Server_EndTurn();

	// ========================================================================
	// UI 通知 (Server → Client RPC)
	// ========================================================================

	/** 顯示訊息 */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "YGO|UI")
	void Client_ShowMessage(const FString &Message);

	/** 請求選擇卡片 (ygopro MSG_SELECT_CARD) */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "YGO|UI")
	void Client_RequestSelectCard(int32 Min, int32 Max, const TArray<AYGOCardActor *> &SelectableCards);

	/** 請求選擇位置 (ygopro MSG_SELECT_PLACE) */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "YGO|UI")
	void Client_RequestSelectPlace(uint8 PlayerID, EYGOLocation Zone);

	/** 請求是/否 (ygopro MSG_SELECT_YESNO) */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "YGO|UI")
	void Client_RequestYesNo(const FString &Question);

	// ========================================================================
	// 決策回應 (Client → Server)
	// ========================================================================

	/** 回應選擇的卡片 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "YGO|Response")
	void Server_RespondSelectCard(const TArray<AYGOCardActor *> &SelectedCards);

	/** 回應選擇的位置 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "YGO|Response")
	void Server_RespondSelectPlace(AYGOFieldZone *SelectedZoneParam);

	/** 回應是/否 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "YGO|Response")
	void Server_RespondYesNo(bool bYes);

	// ========================================================================
	// 事件
	// ========================================================================

	/** 當選擇改變 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, AYGOCardActor *, NewSelection);
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnSelectionChanged OnSelectionChanged;

	/** 當 PlayerState 複製完成 (只在本地 Client 觸發，可在 Blueprint 中綁定) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateReplicated, class AYGOPlayerState *, ReplicatedPlayerState);
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnPlayerStateReplicated OnPlayerStateReplicated;

protected:
	/** Override OnRep_PlayerState 來偵測 PlayerState 複製 (只在 Client 執行) */
	virtual void OnRep_PlayerState() override;

	/** 處理滑鼠點擊 */
	void OnMouseClick();

	/** 處理取消 */
	void OnCancel();

	/** 設定玩家的camera */
	void SetCamera();
};
