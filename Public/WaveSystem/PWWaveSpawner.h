// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PWSpawner.h"
#include "Characters/Player/WaterSystem/PWFountain.h"
#include "GameFramework/Actor.h"
#include "Spawner/PWGridSpawner.h"
#include "GameInstance/PWGlobalEventDispatcher.h"
#include "PWWaveSpawner.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewWaveSignature,int,NewWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNightmareStartSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSpawnSignature,ACharacter*,Boss);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNightmareEndSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPumpkinDestroyed);

UCLASS()
class PROJECTWATER_API APWWaveSpawner : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnNightmareStartSignature OnNightmareStartDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnBossSpawnSignature OnBossSpawnDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnNightmareEndSignature OnNightmareEndDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnPumpkinDestroyed OnPumpkinDestroyedDelegate;

	// Declare On New Wave Delegate
	UPROPERTY(BlueprintAssignable)
	FOnNewWaveSignature OnNewWaveDelegate;
	
	// Sets default values for this actor's properties.
	APWWaveSpawner();

	UPROPERTY(EditAnywhere)
	bool bOverrideDelayBetweenWaves = false;

	UPROPERTY(EditAnywhere,meta=(ClampMin=0,Units="Seconds",EditCondition="bOverrideDelayBetweenWaves"))
	float DelayBetweenWaves = 6;

	UPROPERTY(EditAnywhere)
	int MaxEnemiesSpawnedAtTheSameTime = 30;

	UPROPERTY(BlueprintReadWrite)
	int CurrentWave = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsNightmareWave=false;

	/**
	 * Array of Spawners where the enemies should spawn.
	 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Spawns")
	TArray<TObjectPtr<APWSpawner>> Spawns;

	UPROPERTY(BlueprintReadWrite)
	float PumpkinsBossHealthRetrieve = 0;

	UPROPERTY(BlueprintReadWrite)
	float BossHealth = 0;

	/*
	 * For optimisation purpose, using EQS to register fountains in the game
	 */
	UPROPERTY(EditDefaultsOnly,Category = "Queries")
	UEnvQuery* FountainQuery;

	UPROPERTY(EditAnywhere)
	const TObjectPtr<APWGlobalEventDispatcher> GlobalEventDispatcher = nullptr;
	
	/**
	 * Init a Wave.
	 */
	UFUNCTION(BlueprintCallable)
	void SpawnWave(int Wave);

	/*
	 * Called when a nightmare wave start
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnNightmareStart();

	/*
	 * Called when a nightmare wave end
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnNightmareEnd();

	/*
	 * Used to change manually the current amount of spawned actor, useful when an Actor is glitching out of the map 
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveCurrentSpawnedActor(int amount=1);

	/*
	 * Called when a pumpkin is destroyed and there's no boss spawned, decrease boss initial hp
	 */
	UFUNCTION(BlueprintCallable)
	void OnPumpkinDestroyed(float AmountOfBossHealth);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * Start a Wave for a Spawner.
	 */
	UFUNCTION(BlueprintCallable)
	void StartWave(FTimerHandle& InTimerHandle,APWSpawner* Spawner, FWaveInfo WaveInfo);

	/**
	 * Spawn an Enemy.
	 */
	UFUNCTION()
	void SpawnEnemy(FTimerHandle& TimerHandle,APWSpawner* Spawner,FEnemyInfo EnemyInfo,int count = 1);

	/**
	 * Called when an Actor is Destroyed in the World.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnActorDestroyed();

	/**
	 * Refilling water storage of every player on the game
	 */
	UFUNCTION()
	void RefillWaterForAllPlayer();

	// Called when the Fountain Query is Done
	void FountainQueryFinished(TSharedPtr<FEnvQueryResult> Result);

	/**
	 * Activate the web on the fountains
	 */
	UFUNCTION()
	void ActivateWebOnFountains();

	/**
	 * Desactivate the web on the fountains
	 */
	UFUNCTION()
	void DesactivateWebOnFountains();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APWGridSpawner> SpawnerClass;

	// Used to Make a Query Request For Knowing Every Fountain in the game
	FEnvQueryRequest FountainQueryRequest;

	/**For the progress wave bar*/
	UPROPERTY(EditAnywhere,Category="Health Bar")
	UMaterialInterface* BarMaterialInterface;

	UPROPERTY(BlueprintReadOnly,Category="Health Bar")
	UMaterialInstanceDynamic* BarMaterialInstance;


private:
	UPROPERTY()
	TArray<APWPlayerCharacter*> Players;
	
	UPROPERTY()
	TArray<APWFountain*> Fountains;

	int EnemyMaxInWave = 0;
	int EnemyStillInWave = 0;

	/*
	 * Number of spawner that are active
	 */
	int ActiveSpawners = 0;

	/*
	 * Number of Spawned Actors for the current Wave
	 */
	int currentSpawnedActors = 0;

	/*
	 * Delay before boss and monsters spawn in a nightmare wave
	 */
	int DelayBeforeNightmareStarts = 0;

	/*
	 * Is a nightmare wave actually running
	 */
	bool bIsNightmareWaveRunning = false;

	/*
	 * Create and store dynamically a Timer, return the TimerHandle created.
	 */
	FTimerHandle AddTimerHandle();

	/*
	 * Delete a Timer Handle
	 */
	void RemoveTimerHandle(FTimerHandle& THandle);

	/*
	 * Return true at least one Timer is valid in THandlers
	 */
	bool AreTimersValid() const;

	UFUNCTION()
	void OnSpiderEggSpawned(TArray<AActor*> ActorsSpawned);

	UFUNCTION()
	void OnPumpkinSpawned(TArray<AActor*> ActorsSpawned);

	UFUNCTION()
	void OnGridSpawned(TArray<AActor*> ActorsSpawned);

	UFUNCTION()
	void OnEggDestroyed(AActor* Killer);

	UFUNCTION()
	void OnGridEggHatch(AActor * ActorSpawned);

	UFUNCTION()
	void OnEggHatch(AActor * ActorSpawned);

	void UpdateBarMaterial();

	float PumpkinsBossHealth = 0;

	UPROPERTY()
	TObjectPtr<APWGridSpawner> SpiderSpawner;

	UPROPERTY()
	TObjectPtr<APWGridSpawner> GridSpawner;

	UPROPERTY()
	TObjectPtr<APWGridSpawner> PumpkinSpawner;
	
};
