// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Enemies/PWEnemyCharacter.h"
#include "UObject/Object.h"
#include "PWEnemyData.generated.h"

class UPWEnemyData;

USTRUCT()
struct FEnemyInfo
{
	GENERATED_USTRUCT_BODY()

	/**
	 * Should use a prefab Data Asset
	 */
	UPROPERTY(EditAnywhere)
	bool bOverrideData = false;

	/**
	 * prefab Data to use
	 */
	UPROPERTY(EditAnywhere,meta=(EditCondition="bOverrideData"))
	TObjectPtr<UPWEnemyData> EnemyData = nullptr; 

	/**
	 * Should this enemy use Mass to spawn?
	 */
	UPROPERTY(EditAnywhere, Category="Actor")
	bool bUseMass = false;

	/*
	 * Should this enemy spawn using the random grid
	 */
	UPROPERTY(EditAnywhere,Category="Spawn")
	bool bUseGridSpawn = false;

	/**
	 * The Actor to Spawn
	 */
	UPROPERTY(EditAnywhere, Category="Actor", meta=(EditCondition="!bUseMass&&!bOverrideData"))
	TSubclassOf<AActor> EnemyActor = nullptr;

	/**
	 * The Mass Data Asset to Spawn the Entity
	 */
	UPROPERTY(EditAnywhere, Category="Actor", meta=(EditCondition="bUseMass&&!bOverrideData"))
	TSubclassOf<UDataAsset> EnemyMassDA = nullptr;

	/**
	 * Max Enemies to spawn for the Wave
	 */
	UPROPERTY(EditAnywhere, Category="Amount", meta=(ClampMin=0,EditCondition="!bOverrideData"))
	int MaxSpawnAmount = 1;

	/**
	 * Min Enemies to spawn for the Wave
	 */
	UPROPERTY(EditAnywhere, Category="Amount",meta=(ClampMin=0,EditCondition="!bOverrideData"))
	int MinSpawnAmount = 1;

	/**
	 * Delay before this enemy starts to spawn
	 */

	UPROPERTY(EditAnywhere, Category="Time", meta=(ClampMin=0,Units="Seconds",EditCondition="!bOverrideData"))
	float InitialSpawnDelay = 0;

	/**
	 * Max Spawn delay between each Ennemies
	 */
	UPROPERTY(EditAnywhere, Category="Time", meta=(ClampMin=0,Units="Seconds",EditCondition="!bOverrideData"))
	float MaxSpawnDelay = 1;
	
	/**
	 * Min Spawn delay between each Ennemies
	 */
	UPROPERTY(EditAnywhere, Category="Time", meta=(ClampMin=0,Units="Seconds",EditCondition="!bOverrideData"))
	float MinSpawnDelay = 1;

	
};

/**
 * 
 */
UCLASS()
class PROJECTWATER_API UPWEnemyData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FEnemyInfo Enemy;
};
