// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

UCLASS()
class PROJECTWATER_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when the actor stops playing
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	//Spawn actor of the selected class
	UFUNCTION(BlueprintCallable)
	bool SpawnActor();

	// Change if actors are spawned
	UFUNCTION(BlueprintCallable)
	void EnableActorSpawning(bool Enable);
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bCanSpawn = true;
private:

	// If can still spawn the object, call the spawn actor in this function 
	UFUNCTION()
	void SpawnActorScheduled();

	// Will schedule an actor spawn
	UFUNCTION(BlueprintCallable)
	void StartTheSpawnTimer();
	
	// Box in which we spawn the actor
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBoxComponent> SpawnerBox;

	//Actor to spawn (mostly collectable objects)
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	TSubclassOf<AActor> ActorClassToBeSpawned;

	// Average time between spawns 
	UPROPERTY(EditAnywhere)
	float AverageSpawnTime = 7.0f;
	
	// Random +/- offset of the spawn time
	UPROPERTY(EditAnywhere)
	float RandomSpawnTimeOffset = 1.0f;
	
	// Timer variable
	FTimerHandle SpawnTimerHandle;

};
