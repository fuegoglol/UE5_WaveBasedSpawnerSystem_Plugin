// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "GameFramework/Actor.h"
#include "PWGridSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorsSpawned,TArray<AActor*>, ActorsSpawned);

UCLASS()
class PROJECTWATER_API APWGridSpawner : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Execute(TSubclassOf<AActor> ActorsToSpawn, const int32 Amounts, const bool bRandomRotation);
	
protected:
	APWGridSpawner();

	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UEnvQuery> Query;

	UPROPERTY()
	FEnvQueryRequest QueryRequest;
private:
	UPROPERTY()
	TSubclassOf<AActor> LastActorsToSpawn;

	UPROPERTY()
	int32 LastAmounts = 0;

	UPROPERTY()
	bool bLastRandomRotation = false;
	
	UPROPERTY()
	TArray<FVector> PointsFounds;

	UPROPERTY()
	TArray<AActor*> LastActorsSpawned;
	
	UFUNCTION(BlueprintCallable)
	void FindPoints();

	void OnPointsFound(TSharedPtr<FEnvQueryResult> Result);

	UFUNCTION()
	AActor * Spawn(const FVector & Location) const;

public:
	UPROPERTY(BlueprintAssignable)
	FOnActorsSpawned OnActorsSpawnedDelegate;
};
