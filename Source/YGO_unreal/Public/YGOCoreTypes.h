// YGO_unreal - Yu-Gi-Oh! Game Core Types
// 從 ygopro-core 提取的核心常數和類型定義

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "YGOCoreTypes.generated.h"

// ============================================================================
// 位置 (Locations)
// ============================================================================

UENUM(BlueprintType)
enum class EYGOLocation : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Deck = 1 UMETA(DisplayName = "Deck"),
	Hand = 2 UMETA(DisplayName = "Hand"),
	MonsterZone = 3 UMETA(DisplayName = "Monster Zone"),
	SpellTrapZone = 4 UMETA(DisplayName = "Spell/Trap Zone"),
	Graveyard = 5 UMETA(DisplayName = "Graveyard"),
	Banished = 6 UMETA(DisplayName = "Banished"),
	ExtraDeck = 7 UMETA(DisplayName = "Extra Deck"),
	Overlay = 8 UMETA(DisplayName = "Overlay (XYZ Materials)"),
	FieldZone = 9 UMETA(DisplayName = "Field Spell Zone"),
	PendulumZone = 10 UMETA(DisplayName = "Pendulum Zone")
};

// ============================================================================
// 位置 (Positions)
// ============================================================================

UENUM(BlueprintType)
enum class EYGOPosition : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	FaceUpAttack = 1 UMETA(DisplayName = "Face-up Attack"),
	FaceDownAttack = 2 UMETA(DisplayName = "Face-down Attack"),
	FaceUpDefense = 3 UMETA(DisplayName = "Face-up Defense"),
	FaceDownDefense = 4 UMETA(DisplayName = "Face-down Defense")
};

// ============================================================================
// 卡片類型 (Card Types)
// ============================================================================

// 卡片類型 - 不使用 BlueprintType,改用普通 uint32 來支援位標誌
// 在 Blueprint 中使用 uint32 變數代替

namespace YGOCardType
{
	const uint32 None = 0;
	const uint32 Monster = 0x1;
	const uint32 Spell = 0x2;
	const uint32 Trap = 0x4;
	const uint32 Normal = 0x10;
	const uint32 Effect = 0x20;
	const uint32 Fusion = 0x40;
	const uint32 Ritual = 0x80;
	const uint32 TrapMonster = 0x100;
	const uint32 Spirit = 0x200;
	const uint32 Union = 0x400;
	const uint32 Gemini = 0x800;
	const uint32 Tuner = 0x1000;
	const uint32 Synchro = 0x2000;
	const uint32 Token = 0x4000;
	const uint32 QuickPlay = 0x10000;
	const uint32 Continuous = 0x20000;
	const uint32 Equip = 0x40000;
	const uint32 Field = 0x80000;
	const uint32 Counter = 0x100000;
	const uint32 Flip = 0x200000;
	const uint32 Toon = 0x400000;
	const uint32 Xyz = 0x800000;
	const uint32 Pendulum = 0x1000000;
	const uint32 SpecialSummon = 0x2000000;
	const uint32 Link = 0x4000000;
}

// ============================================================================
// 屬性 (Attributes)
// ============================================================================

UENUM(BlueprintType)
enum class EYGOAttribute : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Earth = 1 UMETA(DisplayName = "EARTH"),
	Water = 2 UMETA(DisplayName = "WATER"),
	Fire = 3 UMETA(DisplayName = "FIRE"),
	Wind = 4 UMETA(DisplayName = "WIND"),
	Light = 5 UMETA(DisplayName = "LIGHT"),
	Dark = 6 UMETA(DisplayName = "DARK"),
	Divine = 7 UMETA(DisplayName = "DIVINE")
};

// ============================================================================
// 種族 (Races)
// ============================================================================

UENUM(BlueprintType)
enum class EYGORace : uint8
{
	Warrior = 0 UMETA(DisplayName = "Warrior"),
	Spellcaster = 1 UMETA(DisplayName = "Spellcaster"),
	Fairy = 2 UMETA(DisplayName = "Fairy"),
	Fiend = 3 UMETA(DisplayName = "Fiend"),
	Zombie = 4 UMETA(DisplayName = "Zombie"),
	Machine = 5 UMETA(DisplayName = "Machine"),
	Aqua = 6 UMETA(DisplayName = "Aqua"),
	Pyro = 7 UMETA(DisplayName = "Pyro"),
	Rock = 8 UMETA(DisplayName = "Rock"),
	WingedBeast = 9 UMETA(DisplayName = "Winged Beast"),
	Plant = 10 UMETA(DisplayName = "Plant"),
	Insect = 11 UMETA(DisplayName = "Insect"),
	Thunder = 12 UMETA(DisplayName = "Thunder"),
	Dragon = 13 UMETA(DisplayName = "Dragon"),
	Beast = 14 UMETA(DisplayName = "Beast"),
	BeastWarrior = 15 UMETA(DisplayName = "Beast-Warrior"),
	Dinosaur = 16 UMETA(DisplayName = "Dinosaur"),
	Fish = 17 UMETA(DisplayName = "Fish"),
	SeaSerpent = 18 UMETA(DisplayName = "Sea Serpent"),
	Reptile = 19 UMETA(DisplayName = "Reptile"),
	Psychic = 20 UMETA(DisplayName = "Psychic"),
	DivineBeast = 21 UMETA(DisplayName = "Divine-Beast"),
	CreatorGod = 22 UMETA(DisplayName = "Creator God"),
	Wyrm = 23 UMETA(DisplayName = "Wyrm"),
	Cyberse = 24 UMETA(DisplayName = "Cyberse"),
	Illusion = 25 UMETA(DisplayName = "Illusion")
};

// ============================================================================
// 回合階段 (Phases)
// ============================================================================

UENUM(BlueprintType)
enum class EYGOPhase : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Draw = 1 UMETA(DisplayName = "Draw Phase"),
	Standby = 2 UMETA(DisplayName = "Standby Phase"),
	Main1 = 3 UMETA(DisplayName = "Main Phase 1"),
	BattleStart = 4 UMETA(DisplayName = "Battle Start"),
	BattleStep = 5 UMETA(DisplayName = "Battle Step"),
	Damage = 6 UMETA(DisplayName = "Damage Step"),
	DamageCalculation = 7 UMETA(DisplayName = "Damage Calculation"),
	Battle = 8 UMETA(DisplayName = "Battle Phase"),
	Main2 = 9 UMETA(DisplayName = "Main Phase 2"),
	End = 10 UMETA(DisplayName = "End Phase")
};

// ============================================================================
// 遊戲訊息類型 (Message Types)
// ============================================================================

UENUM(BlueprintType)
enum class EYGOMessage : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Retry = 1 UMETA(DisplayName = "Retry"),
	Hint = 2 UMETA(DisplayName = "Hint"),
	Win = 5 UMETA(DisplayName = "Win"),
	SelectBattleCmd = 10 UMETA(DisplayName = "Select Battle Command"),
	SelectIdleCmd = 11 UMETA(DisplayName = "Select Idle Command"),
	SelectEffectYN = 12 UMETA(DisplayName = "Select Effect Yes/No"),
	SelectYesNo = 13 UMETA(DisplayName = "Select Yes/No"),
	SelectOption = 14 UMETA(DisplayName = "Select Option"),
	SelectCard = 15 UMETA(DisplayName = "Select Card"),
	SelectChain = 16 UMETA(DisplayName = "Select Chain"),
	SelectPlace = 18 UMETA(DisplayName = "Select Place"),
	SelectPosition = 19 UMETA(DisplayName = "Select Position"),
	SelectTribute = 20 UMETA(DisplayName = "Select Tribute"),
	NewTurn = 40 UMETA(DisplayName = "New Turn"),
	NewPhase = 41 UMETA(DisplayName = "New Phase"),
	Move = 50 UMETA(DisplayName = "Move Card"),
	PosChange = 53 UMETA(DisplayName = "Position Change"),
	Set = 54 UMETA(DisplayName = "Set Card"),
	Summoning = 60 UMETA(DisplayName = "Summoning"),
	Summoned = 61 UMETA(DisplayName = "Summoned"),
	SpSummoning = 62 UMETA(DisplayName = "Special Summoning"),
	SpSummoned = 63 UMETA(DisplayName = "Special Summoned"),
	Chaining = 70 UMETA(DisplayName = "Chaining Effect"),
	Chained = 71 UMETA(DisplayName = "Chained"),
	Draw = 90 UMETA(DisplayName = "Draw"),
	Damage = 91 UMETA(DisplayName = "Damage"),
	Recover = 92 UMETA(DisplayName = "Recover LP"),
	LPUpdate = 94 UMETA(DisplayName = "LP Update"),
	Attack = 110 UMETA(DisplayName = "Attack"),
	Battle = 111 UMETA(DisplayName = "Battle")
};

// ============================================================================
// 卡片資料結構 (對應 CSV 的 STR_CardData)
// ============================================================================

USTRUCT(BlueprintType)
struct YGO_UNREAL_API FYGOCardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 CardCode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FString CardName;

	// 卡片類型 (uint32 不支援 Blueprint,僅 C++ 使用)
	UPROPERTY(EditAnywhere, Category = "Card Data")
	uint32 Type = 0; // 使用 YGOCardType namespace 的位標誌

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	EYGOAttribute Attribute = EYGOAttribute::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	EYGORace Race = EYGORace::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 Level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 Attack = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 Defense = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FString Description;

	// Link 怪獸
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data|Link")
	int32 LinkValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data|Link")
	int32 LinkMarkers = 0;

	// Pendulum 怪獸
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data|Pendulum")
	int32 LeftScale = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data|Pendulum")
	int32 RightScale = 0;
};

// ============================================================================
// 卡片資料結構 (對應 CSV 的 STR_SpriteSheetIndex)
// ============================================================================

USTRUCT(BlueprintType)
struct YGO_UNREAL_API FYGOSpriteSheetIndex : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 Index = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FString Comment;
};

// ============================================================================
// 卡片資料結構 (對應 CSV 的 STR_DeckRow)
// ============================================================================

USTRUCT(BlueprintType)
struct YGO_UNREAL_API FYGODeckRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 cardId = -1;
};

// ============================================================================
// 卡片資料結構 (對應 CSV 的 STR_CardText)
// ============================================================================

USTRUCT(BlueprintType)
struct YGO_UNREAL_API FYGOCardTextRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FString jp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FString en;
};

// ============================================================================
// 卡片實例 (運行時狀態)
// ============================================================================

USTRUCT(BlueprintType)
struct YGO_UNREAL_API FYGOCardInstance
{
	GENERATED_BODY()

	// 卡片靜態資料
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
	FYGOCardData CardData;

	// 運行時狀態
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|State")
	int32 InstanceID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|State")
	uint8 OwnerPlayerID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|State")
	uint8 ControllerPlayerID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|State")
	EYGOLocation Location = EYGOLocation::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|State")
	uint8 Sequence = 0; // 位置索引 (0-4 for zones)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|State")
	EYGOPosition Position = EYGOPosition::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|State")
	int32 CurrentAttack = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|State")
	int32 CurrentDefense = 0;
};
