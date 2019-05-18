// Fill out your copyright notice in the Description page of Project Settings.

#include "Teleporter.h"

// Sets default values
ATeleporter::ATeleporter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Components
	sphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	sphereComponent->SetupAttachment(RootComponent);
	sphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);	//so that it generates hit events
	sphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ATeleporter::OnOverlapBegin);



}

// Called when the game starts or when spawned
void ATeleporter::BeginPlay()
{
	Super::BeginPlay();

	//set overlap function
	sphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ATeleporter::OnOverlapBegin);
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Begin")));
}


// Called every frame
void ATeleporter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void ATeleporter::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	//See if player overlapped.
	//If so, teleport them to destination
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Overlapped Teleporter")));

	ASurvios_2019_01Character *playerChar = Cast<ASurvios_2019_01Character>(OtherActor);

	if (playerChar != nullptr)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Player Character Got")));
		TeleportPlayer(playerChar);
	}
}

void ATeleporter::TeleportPlayer(ASurvios_2019_01Character * playerChar)
{

	//figure out where to send them
	FVector destination;
	if (target == nullptr)
		destination = GetActorLocation();
	else
		destination = target->GetActorLocation();
	//And away they go!
	playerChar->SetActorLocation(destination);
	//set checkpoint for player
	playerChar->spawnPoint = target;
}

