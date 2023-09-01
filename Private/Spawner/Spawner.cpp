// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawner/Spawner.h"
#include "TimerManager.h"

// Sets default values
ASpawner::ASpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//setup
	SpawnerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawner box"));
	RootComponent = SpawnerBox;
}

// Called when the game starts or when spawned
void ASpawner::BeginPlay()
{
	Super::BeginPlay();
	
	//Schedule first spawn
	if(bCanSpawn == true)
	{
		StartTheSpawnTimer();
	}
}

void ASpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
    
	// Remove all timers associated with this objects instance
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

bool ASpawner::SpawnActor()
{
	bool SpawnedActor = false;
	if (ActorClassToBeSpawned)
	{
		// Calculate the extends of the box
		FBoxSphereBounds BoxBounds = SpawnerBox->CalcBounds(GetActorTransform());

		// Compute a random position within the box bounds
		FVector SpawnLocation = BoxBounds.Origin;
		SpawnLocation.X += -BoxBounds.BoxExtent.X + 2 * BoxBounds.BoxExtent.X * FMath::FRand();
		SpawnLocation.Y += -BoxBounds.BoxExtent.Y + 2 * BoxBounds.BoxExtent.Y * FMath::FRand();
		SpawnLocation.Z += -BoxBounds.BoxExtent.Z + 2 * BoxBounds.BoxExtent.Z * FMath::FRand();

		// Spawn the actor
		SpawnedActor = GetWorld()->SpawnActor(ActorClassToBeSpawned, &SpawnLocation) != nullptr;
	}

	return SpawnedActor;
}

void ASpawner::EnableActorSpawning(bool Enable)
{
	// Update internal state
	bCanSpawn = Enable;

	// Check witch timer action should be done
	if (Enable)
	{
		StartTheSpawnTimer();
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void ASpawner::StartTheSpawnTimer()
{
	//Compute time offset to spawn
	const float DeltaToNextSpawn = AverageSpawnTime + (-RandomSpawnTimeOffset + 2 * RandomSpawnTimeOffset * FMath::FRand());

	//Schedule spawning
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &ASpawner::SpawnActorScheduled, DeltaToNextSpawn, false);
}

void ASpawner::SpawnActorScheduled()
{
	// Spawn and Reschedule if required
	if (SpawnActor() == true) //if the actor has been spawned
	{	
		if (bCanSpawn == true)
		{
			StartTheSpawnTimer();
		}
	}
	else
	{
		//Spawn actor returned false
	}
}