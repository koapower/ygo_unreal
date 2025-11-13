#include "YGOFieldZone.h"
#include "YGOCardActor.h"
#include "YGOGameState.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

// YGO_unreal - Field Zone Implementation
AYGOFieldZone::AYGOFieldZone()
{
	PrimaryActorTick.bCanEverTick = false;

	PlayerID = 0;
	ZoneType = EYGOLocation::MonsterZone;
	Sequence = 0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	// 創建視覺化組件 (方形平面,用於在編輯器中顯示位置)
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);

	// 載入簡單的立方體網格
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		VisualMesh->SetStaticMesh(CubeMeshAsset.Object);
		VisualMesh->SetWorldScale3D(FVector(1.0f, 0.7f, 0.05f)); // 扁平方塊
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AYGOFieldZone::BeginPlay()
{
	Super::BeginPlay();

	// 遊戲開始後是否隱藏視覺化網格
	if (VisualMesh)
	{
		VisualMesh->SetVisibility(showMesh);
	}
}

#if WITH_EDITOR
void AYGOFieldZone::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	// 根據區域類型改變顏色
	if (VisualMesh)
	{
		VisualMesh->SetVisibility(true);

		// 根據玩家和區域類型設定不同顏色
		// 這只是編輯器預覽,遊戲中會隱藏
	}
}
#endif

void AYGOFieldZone::AddCard(AYGOCardActor *Card)
{
	if (!Card || CardsInZone.Contains(Card))
	{
		return;
	}

	CardsInZone.Add(Card);
	UpdateCardPositions();

	//UE_LOG(LogTemp, Log, TEXT("[FieldZone] Added card to zone. Total cards: %d"), CardsInZone.Num());
}

void AYGOFieldZone::RemoveCard(AYGOCardActor *Card)
{
	if (!Card)
	{
		return;
	}

	CardsInZone.Remove(Card);
	UpdateCardPositions();

	//UE_LOG(LogTemp, Log, TEXT("[FieldZone] Removed card from zone. Remaining cards: %d"), CardsInZone.Num());
}

void AYGOFieldZone::UpdateCardPositions()
{
	for (int32 i = 0; i < CardsInZone.Num(); ++i)
	{
		if (CardsInZone[i])
		{
			CardsInZone[i]->StackIndex = i;
			CardsInZone[i]->UpdateTargetTransform();
		}
	}
}

FVector AYGOFieldZone::GetCardLocalOffset(int32 CardIndex) const
{
	switch (ZoneType)
	{
	case EYGOLocation::Deck:
	case EYGOLocation::ExtraDeck:
	case EYGOLocation::Graveyard:
	case EYGOLocation::Banished:
		// 垂直堆疊
		return FVector(0, 0, CardIndex * 0.5f + 3.0f);

	case EYGOLocation::Hand:
		// 橫向扇形排列
		{
			int32 TotalCards = CardsInZone.Num();
			float Spacing = 80.0f;
			float StartOffset = -(TotalCards - 1) * Spacing * 0.5f;
			return FVector(0, StartOffset + CardIndex * Spacing, CardIndex * 0.1f);
		}

	case EYGOLocation::MonsterZone:
	case EYGOLocation::SpellTrapZone:
	case EYGOLocation::FieldZone:
	case EYGOLocation::PendulumZone:
		// 場地格子，單卡位置 (略微抬高避免 Z-fighting)
		return FVector(0, 0, CardIndex * 0.5f + 1.0f);

	default:
		return FVector::ZeroVector;
	}
}

// 舊的 API - 向後兼容
void AYGOFieldZone::PlaceCard(AYGOCardActor *Card)
{
	AddCard(Card);
}
