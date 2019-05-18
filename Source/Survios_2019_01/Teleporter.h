// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Survios_2019_01Character.h"
#include "Components/SphereComponent.h"
#include "Teleporter.generated.h"

UCLASS()
class SURVIOS_2019_01_API ATeleporter : public AActor
{
	GENERATED_BODY()

private:
	USphereComponent *sphereComponent;

public:	
	// Sets default values for this actor's properties
	ATeleporter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor *target;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//When we overlap
	UFUNCTION()		//Don't forget this line, or else it won't work.... ;)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BluePrintCallable)
	void TeleportPlayer(ASurvios_2019_01Character * playerChar);
};
