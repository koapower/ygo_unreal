// YGO_unreal - Card Actor Implementation

#include "YGOCardActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "YGOPlayerState.h"

AYGOCardActor::AYGOCardActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 啟用網路複製
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// 創建卡片網格組件
	CardFrontMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardFrontMesh"));
	CardFrontMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		CardFrontMesh->SetStaticMesh(PlaneMesh.Object);
		CardFrontMesh->SetRelativeScale3D(FVector(0.484f, 0.7f, 1.0f));
	}

	CardBackMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardBackMesh"));
	CardBackMesh->SetupAttachment(RootComponent);

	if (PlaneMesh.Succeeded())
	{
		CardBackMesh->SetStaticMesh(PlaneMesh.Object);
		CardBackMesh->SetRelativeScale3D(FVector(0.484f, 0.7f, 1.0f));

		// 翻轉180度 (讓它背對正面)
		CardBackMesh->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
		CardBackMesh->SetRelativeLocation(FVector(0.f, 0.f, -0.01f)); // 微微偏移避免 Z-fighting
	}

	// 啟用點擊事件
	CardFrontMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CardFrontMesh->SetCollisionResponseToAllChannels(ECR_Block);
	CardFrontMesh->SetGenerateOverlapEvents(true);
	CardFrontMesh->OnClicked.AddDynamic(this, &AYGOCardActor::OnClicked);
	CardBackMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CardBackMesh->SetCollisionResponseToAllChannels(ECR_Block);
	CardBackMesh->SetGenerateOverlapEvents(true);
	CardBackMesh->OnClicked.AddDynamic(this, &AYGOCardActor::OnClicked);

	// 預設狀態
	bFaceUp = false;
	CardFrontMaterialTemplate = nullptr;
	CardBackMaterialTemplate = nullptr;
	DynamicFrontMaterial = nullptr;
	DynamicBackMaterial = nullptr;
}

void AYGOCardActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYGOCardActor, CardInstance);
	DOREPLIFETIME(AYGOCardActor, bFaceUp);
}

void AYGOCardActor::BeginPlay()
{
	Super::BeginPlay();

	// 創建動態材質實例
	if (CardFrontMaterialTemplate)
	{
		DynamicFrontMaterial = UMaterialInstanceDynamic::Create(CardFrontMaterialTemplate, this);
		if (DynamicFrontMaterial)
		{
			CardFrontMesh->SetMaterial(0, DynamicFrontMaterial);
		}
	}

	if (CardBackMaterialTemplate)
	{
		DynamicBackMaterial = UMaterialInstanceDynamic::Create(CardBackMaterialTemplate, this);
		if (DynamicBackMaterial)
		{
			CardBackMesh->SetMaterial(0, DynamicBackMaterial);
		}
	}

	// 初始化視覺效果
	UpdateCardVisual();
}

void AYGOCardActor::OnRep_CardInstance()
{
	// 當卡片資料複製到客戶端時更新視覺效果
	UpdateCardVisual();
}

void AYGOCardActor::OnRep_FaceUp()
{
	// 當正反面狀態改變時更新視覺效果
	UpdateCardVisual();
}

void AYGOCardActor::SetCardData(const FYGOCardInstance &NewCardData)
{
	if (!HasAuthority())
	{
		return;
	}

	CardInstance = NewCardData;
	UpdateCardVisual();
}

void AYGOCardActor::UpdateCardVisual()
{
	if (!CardFrontMesh || !CardBackMesh)
	{
		return;
	}

	if (!SpriteSheetDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpriteSheetDataTable is null"));
		return;
	}

	FName RowName = FName(*FString::FromInt(CardInstance.CardData.CardCode));
	const FYGOSpriteSheetIndex *Row = SpriteSheetDataTable->FindRow<FYGOSpriteSheetIndex>(
		RowName, TEXT("CardVisualLookup"));

	bool bShouldShowFront = bFaceUp || IsOwnedByLocalPlayer();
	if (Row)
	{
		if (DynamicFrontMaterial && bShouldShowFront)
		{
			DynamicFrontMaterial->SetScalarParameterValue(TEXT("Index"), Row->Index);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No SpriteSheetIndex found for CardCode: %d"), CardInstance.CardData.CardCode);
		return;
	}

	// 根據位置設定旋轉
	if (CardInstance.Position == EYGOPosition::FaceUpDefense ||
		CardInstance.Position == EYGOPosition::FaceDownDefense)
	{
		// 守備表示 - 橫向
		SetActorRotation(FRotator(0, 0, 90));
	}
	else
	{
		// 攻擊表示 - 縱向
		SetActorRotation(FRotator(0, 0, 0));
	}
}

void AYGOCardActor::SetCardArtIndex(int32 Index)
{
	if (DynamicFrontMaterial)
	{
		DynamicFrontMaterial->SetScalarParameterValue(FName("Index"), static_cast<float>(Index));
	}
}

void AYGOCardActor::FlipFaceUp()
{
	if (!HasAuthority())
	{
		return;
	}

	bFaceUp = true;
	CardInstance.Position = EYGOPosition::FaceUpAttack;
	UpdateCardVisual();
}

void AYGOCardActor::FlipFaceDown()
{
	if (!HasAuthority())
	{
		return;
	}

	bFaceUp = false;
	CardInstance.Position = EYGOPosition::FaceDownDefense;
	UpdateCardVisual();
}

void AYGOCardActor::SetCardPosition(EYGOPosition NewPosition)
{
	if (!HasAuthority())
	{
		return;
	}

	CardInstance.Position = NewPosition;

	// 更新正反面狀態
	switch (NewPosition)
	{
	case EYGOPosition::FaceUpAttack:
	case EYGOPosition::FaceUpDefense:
		bFaceUp = true;
		break;

	case EYGOPosition::FaceDownAttack:
	case EYGOPosition::FaceDownDefense:
		bFaceUp = false;
		break;

	default:
		break;
	}

	UpdateCardVisual();
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

void AYGOCardActor::AttackCard(AYGOCardActor *Target)
{
	if (!HasAuthority() || !Target)
	{
		return;
	}

	// TODO: 實作戰鬥邏輯
	UE_LOG(LogTemp, Log, TEXT("[Card] %s attacks %s"),
		   *CardInstance.CardData.CardName,
		   *Target->CardInstance.CardData.CardName);
}

void AYGOCardActor::DirectAttack()
{
	if (!HasAuthority())
	{
		return;
	}

	// TODO: 實作直接攻擊邏輯
	UE_LOG(LogTemp, Log, TEXT("[Card] %s direct attack!"),
		   *CardInstance.CardData.CardName);
}

bool AYGOCardActor::CanAttack() const
{
	// TODO: 檢查是否可以攻擊
	return IsOnField() &&
		   bFaceUp &&
		   (CardInstance.Position == EYGOPosition::FaceUpAttack);
}

void AYGOCardActor::OnClicked(UPrimitiveComponent *TouchedComponent, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Log, TEXT("[Card] Clicked: %s"), *CardInstance.CardData.CardName);

	if (OnCardClicked.IsBound())
	{
		OnCardClicked.Broadcast(this);
	}
}

bool AYGOCardActor::IsOwnedByLocalPlayer() const
{
	// 檢查這張卡是否屬於本地玩家
	APlayerController *LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC)
	{
		return false;
	}

	AYGOPlayerState *LocalPlayerState = LocalPC->GetPlayerState<AYGOPlayerState>();
	if (!LocalPlayerState)
	{
		return false;
	}

	return CardInstance.OwnerPlayerID == LocalPlayerState->YGOPlayerID;
}
