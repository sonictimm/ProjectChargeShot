// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyClass.h"
#include "Engine/Classes/Components/CapsuleComponent.h"
#include "Survios_2019_01Character.h"
#include "TimerManager.h"
#include "Damager.h"

// Sets default values
AEnemyClass::AEnemyClass()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Default health and damage
	maxHealth = 40;
	dmgContact = 20;
	invincible = false;
	invincibilityTime = (float).5f;
}

// Called when the game starts or when spawned
void AEnemyClass::BeginPlay()
{
	Super::BeginPlay();
	SetHealth(maxHealth);

	//Enable hit/overlap events
	//Enable hit events for the capsule
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AEnemyClass::OnCompHit);
	//set overlap function, which calls OnComponentHit
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AEnemyClass::OnOverlapBegin);
}

void AEnemyClass::EndInvincibility()
{
	invincible = false;
}

// Called every frame
void AEnemyClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyClass::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
void AEnemyClass::SetHealth(float newHealth)
{
	//Ensure health is above zero, or kill thing
	if (newHealth <= 0)
	{
		health = 0;
		DeathSequence();
	}
	//ensure health  isn't greater than max
	else if (newHealth > maxHealth)
		health = maxHealth;
	else
		health = newHealth;	//assign new health value
}
void AEnemyClass::TakeDamage(float damage)
{
	//invincibility		
	if (invincible)
		return;
	//Healing
	else if ((health - damage) >= maxHealth)
	{
		health = maxHealth;
	}
	//Handle Dying
	else if (damage >= health)
	{
		health = 0;
		DeathSequence();

	}
	//normally take damage
	else
	{
		health -= damage;
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Enemy Got Damaged: %f"), damage));
		invincible = true;
		GetWorldTimerManager().SetTimer(invincibilityTimer, this, &AEnemyClass::EndInvincibility, invincibilityTime, false, -1.f);
	}
}
void AEnemyClass::DeathSequence()
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Enemy Got Dead")));
	//turn off damage and collisions
	dmgContact = 0;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AEnemyClass::OnCompHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Overlapped by: %s %s"), *OtherActor->GetName(), *OtherComp->GetName()  ));

	//if other thing is damager, take damage
	if (Cast<ADamager>(OtherActor) != nullptr)
	{
		TakeDamage(Cast<ADamager>(OtherActor)->damage);
		Cast<ADamager>(OtherActor)->HitObject(this);
	}
	//even if it's just a comopnent
	else if (Cast<ADamager>(OtherComp) != nullptr)
	{
		TakeDamage(Cast<ADamager>(OtherComp)->damage);
		Cast<ADamager>(OtherActor)->HitObject(this);
	}
	//if other thing is a player, respond to that
	else if (Cast<ASurvios_2019_01Character>(OtherActor) != nullptr)
	{
		Cast<ASurvios_2019_01Character>(OtherActor)->CharacterHit(this);
	}
	//if other thing is another enemy, do nothing

}

void AEnemyClass::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	OnCompHit(OverlappedComp, OtherActor, OtherComp, FVector(0.f, 0.f, 0.f), SweepResult);
}

