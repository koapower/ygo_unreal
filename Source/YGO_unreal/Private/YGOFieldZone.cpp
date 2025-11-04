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
	bOccupied = false;
	OccupyingCard = nullptr;

	// 創建視覺化組件 (方形平面,用於在編輯器中顯示位置)
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	RootComponent = VisualMesh;

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

	// 自動註冊到 GameState
	// AYGOGameState* GameState = GetWorld()->GetGameState<AYGOGameState>();
	// if (GameState && HasAuthority())
	// {
	// 	GameState->RegisterFieldZone(this);
	// }

	// 遊戲開始後隱藏視覺化網格 (只在編輯器中需要看到)
	if (VisualMesh)
	{
		VisualMesh->SetVisibility(true);
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

void AYGOFieldZone::PlaceCard(AYGOCardActor *Card)
{
	if (!Card)
	{
		return;
	}

	OccupyingCard = Card;
	bOccupied = true;

	// 將卡片移動到此位置
	Card->SetActorLocation(GetActorLocation() + FVector(0, 0, 10)); // 稍微抬高避免Z-fighting
	Card->SetActorRotation(GetActorRotation());
}

void AYGOFieldZone::RemoveCard()
{
	OccupyingCard = nullptr;
	bOccupied = false;
}
