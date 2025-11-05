#include "YGODataTableSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

void UYGODataTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadAllDataTablesFromFolder();
}

void UYGODataTableSubsystem::Deinitialize()
{
	LoadedDataTables.Empty();
	Super::Deinitialize();
}

void UYGODataTableSubsystem::LoadAllDataTablesFromFolder()
{
	LoadedDataTables.Empty();

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UDataTable::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(TEXT("/Game/Table"));
	Filter.bRecursivePaths = true;

	AssetRegistry.GetAssets(Filter, AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UDataTable* DataTable = Cast<UDataTable>(AssetData.GetAsset());
		if (DataTable)
		{
			FName TableName = FName(*AssetData.AssetName.ToString());
			LoadedDataTables.Add(TableName, DataTable);
		}
	}
}

UDataTable* UYGODataTableSubsystem::GetDataTable(FName TableName) const
{
	if (UDataTable* const* FoundTable = LoadedDataTables.Find(TableName))
	{
		return *FoundTable;
	}
	return nullptr;
}

TArray<FName> UYGODataTableSubsystem::GetAllDataTableNames() const
{
	TArray<FName> Names;
	LoadedDataTables.GetKeys(Names);
	return Names;
}

bool UYGODataTableSubsystem::HasDataTable(FName TableName) const
{
	return LoadedDataTables.Contains(TableName);
}
