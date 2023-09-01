// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawner/PWGridSpawner.h"
#include "MathUtil.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
APWGridSpawner::APWGridSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APWGridSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

void APWGridSpawner::Execute(TSubclassOf<AActor> ActorsToSpawn, const int32 Amounts, const bool bRandomRotation)
{
	LastActorsToSpawn = ActorsToSpawn;
	LastAmounts = Amounts;
	bLastRandomRotation = bRandomRotation;
	FindPoints();	
}

void APWGridSpawner::FindPoints()
{
	QueryRequest = FEnvQueryRequest(Query, this);
	QueryRequest.Execute(EEnvQueryRunMode::AllMatching, this, &ThisClass::OnPointsFound);
}

void APWGridSpawner::OnPointsFound(TSharedPtr<FEnvQueryResult> Result)
{
	LastActorsSpawned.Reset();
	Result->GetAllAsLocations(PointsFounds);
	if(PointsFounds.Num() == 0) return;

	int32 ActorsSpawned = 0;
	while(ActorsSpawned < LastAmounts)
	{
		const int32 Index = FMath::RandRange(0, PointsFounds.Num()-1);
		const FVector & Location = PointsFounds[Index];
		LastActorsSpawned.Add(Spawn(Location));
		PointsFounds.RemoveAt(Index);
		++ActorsSpawned;
	}
	OnActorsSpawnedDelegate.Broadcast(LastActorsSpawned);
}

AActor * APWGridSpawner::Spawn(const FVector& Location) const
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	FRotator CurrentRotation = FRotator::ZeroRotator;
	if(bLastRandomRotation)
	{
		const int32 Yaw = FMath::RandRange(0, 360);
		CurrentRotation = FRotator(0,Yaw,0);		
	}
		
	return GetWorld()->SpawnActor<AActor>(LastActorsToSpawn,Location,CurrentRotation,SpawnParameters);
}



