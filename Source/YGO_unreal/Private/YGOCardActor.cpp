// YGO_unreal - Card Actor Implementation

#include "YGOCardActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AYGOCardActor::AYGOCardActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 啟用網路複製
	bReplicates = true;
	SetReplicates(true);

	// 創建卡片網格組件
	CardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardMesh"));
	RootComponent = CardMesh;

	// 載入卡片平面網格 (使用 Engine 內建的 Plane)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		CardMesh->SetStaticMesh(PlaneMesh.Object);
		CardMesh->SetWorldScale3D(FVector(0.63f, 0.88f, 1.0f)); // 遊戲王卡片比例 (63mm x 88mm)
		CardMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CardMesh->SetCollisionResponseToAllChannels(ECR_Block);
	}

	// 啟用點擊事件
	CardMesh->SetGenerateOverlapEvents(true);
	CardMesh->OnClicked.AddDynamic(this, &AYGOCardActor::HandleClicked);
}

void AYGOCardActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYGOCardActor, CardInstance);
}

void AYGOCardActor::BeginPlay()
{
	Super::BeginPlay();

	// 創建動態材質實例
	if (CardMesh && CardMesh->GetMaterial(0))
	{
		CardMaterial = CardMesh->CreateDynamicMaterialInstance(0);
	}

	UpdateCardVisual();
}

void AYGOCardActor::OnRep_CardInstance()
{
	// 當卡片資料在網路上更新時,更新視覺
	UpdateCardVisual();
}

void AYGOCardActor::SetCardData(const FYGOCardInstance& NewCardData)
{
	if (!HasAuthority())
	{
		return; // 只有 Server 可以設定
	}

	CardInstance = NewCardData;
	UpdateCardVisual();
}

void AYGOCardActor::UpdateCardVisual()
{
	if (!CardMesh || !CardMaterial)
	{
		return;
	}

	// TODO: 根據 CardInstance.CardData.CardCode 設定卡圖
	// 這裡需要連接到你的 sprite atlas 系統
	// 參考你的 M_CardAtlas material

	// 根據位置設定旋轉
	SetCardPosition(CardInstance.Position);

	// 更新攻擊力/守備力顯示 (如果有 UI 組件)
	// ...
}

void AYGOCardActor::SetCardPosition(EYGOPosition NewPosition)
{
	CardInstance.Position = NewPosition;

	FRotator NewRotation = GetActorRotation();

	switch (NewPosition)
	{
	case EYGOPosition::FaceUpAttack:
		// 正面朝上,直立
		NewRotation.Pitch = 0;
		NewRotation.Roll = 0;
		break;

	case EYGOPosition::FaceDownAttack:
		// 背面朝上,直立
		NewRotation.Pitch = 0;
		NewRotation.Roll = 180;
		break;

	case EYGOPosition::FaceUpDefense:
		// 正面朝上,橫置
		NewRotation.Pitch = 0;
		NewRotation.Roll = 0;
		NewRotation.Yaw += 90;
		break;

	case EYGOPosition::FaceDownDefense:
		// 背面朝上,橫置
		NewRotation.Pitch = 0;
		NewRotation.Roll = 180;
		NewRotation.Yaw += 90;
		break;
	}

	SetActorRotation(NewRotation);
}

void AYGOCardActor::FlipCard()
{
	// 翻面
	bool bCurrentlyFaceUp = IsFaceUp();

	if (bCurrentlyFaceUp)
	{
		// 變成裡側
		if (IsAttackPosition())
		{
			SetCardPosition(EYGOPosition::FaceDownAttack);
		}
		else
		{
			SetCardPosition(EYGOPosition::FaceDownDefense);
		}
	}
	else
	{
		// 變成表側
		if (IsAttackPosition())
		{
			SetCardPosition(EYGOPosition::FaceUpAttack);
		}
		else
		{
			SetCardPosition(EYGOPosition::FaceUpDefense);
		}
	}
}

bool AYGOCardActor::IsMonster() const
{
	return (CardInstance.CardData.Type & YGOCardType::Monster) != 0;
}

bool AYGOCardActor::IsSpell() const
{
	return (CardInstance.CardData.Type & YGOCardType::Spell) != 0;
}

bool AYGOCardActor::IsTrap() const
{
	return (CardInstance.CardData.Type & YGOCardType::Trap) != 0;
}

bool AYGOCardActor::IsOnField() const
{
	return CardInstance.Location == EYGOLocation::MonsterZone ||
	       CardInstance.Location == EYGOLocation::SpellTrapZone ||
	       CardInstance.Location == EYGOLocation::FieldZone;
}

bool AYGOCardActor::IsFaceUp() const
{
	return CardInstance.Position == EYGOPosition::FaceUpAttack ||
	       CardInstance.Position == EYGOPosition::FaceUpDefense;
}

bool AYGOCardActor::IsAttackPosition() const
{
	return CardInstance.Position == EYGOPosition::FaceUpAttack ||
	       CardInstance.Position == EYGOPosition::FaceDownAttack;
}

void AYGOCardActor::AttackCard(AYGOCardActor* Target)
{
	if (!HasAuthority() || !Target)
	{
		return;
	}

	// TODO: 實作戰鬥邏輯
	// 這裡應該呼叫 GameMode 或 GameState 的戰鬥處理函數

	UE_LOG(LogTemp, Log, TEXT("[Card] %d attacks %d!"),
	       CardInstance.InstanceID, Target->CardInstance.InstanceID);
}

void AYGOCardActor::DirectAttack()
{
	if (!HasAuthority())
	{
		return;
	}

	// TODO: 實作直接攻擊邏輯
	UE_LOG(LogTemp, Log, TEXT("[Card] %d direct attack!"), CardInstance.InstanceID);
}

bool AYGOCardActor::CanAttack() const
{
	// 檢查是否可以攻擊
	// TODO: 加入更多條件判斷 (是否在自己回合、是否已攻擊過等)
	return IsMonster() && IsOnField() && IsAttackPosition() && IsFaceUp();
}

void AYGOCardActor::HandleClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	// 觸發點擊事件
	if (OnCardClicked.IsBound())
	{
		OnCardClicked.Broadcast(this);
	}

	UE_LOG(LogTemp, Log, TEXT("[Card] Clicked: %s"), *CardInstance.CardData.CardName);
}
