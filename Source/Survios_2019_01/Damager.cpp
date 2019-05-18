// Fill out your copyright notice in the Description page of Project Settings.

#include "Damager.h"
#include "IKillableEntity.h"
#include "Survios_2019_01Character.h"
#include "EnemyClass.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
//Common among constructors:
void ADamager::InitializeComponents()
{
	//manage basic components
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComp->ResetRelativeTransform();
	SetRootComponent(RootComp);
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
	//CapsuleComponent->AttachToComponent(RootComp,FAttachmentTransformRules::SnapToTargetNotIncludingScale);	//does the same as below line
	CapsuleComponent->SetupAttachment(RootComp);
	CapsuleComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	CapsuleComponent->SetRelativeLocation(FVector(0.f, 0.f, 22.f));
	CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);	//so that it generates hit events
	//enable tick
	PrimaryActorTick.bCanEverTick = true;
	
}
ADamager::ADamager()
{
	//Deal with basic components
	InitializeComponents();

	//assign defaults:
	friendlyFire = false;
	damage = 10;
	//how many objects it overlaps before de-spawning
	persistence = 1;
	//how much it moves each frame
	displacement = FVector(0.f, 0.f,50.f);

	//who fired this or owns this:
	//damagerOwner = nullptr;


}

ADamager::ADamager(AActor *dOwner, float damage, int persistence, FVector displacement, bool friendlyFire)
{
	//Setup regardless of inputs:
	InitializeComponents();
	//assign parameters:
	this->damagerOwner = dOwner;
	this->damage = damage;
	this->persistence = persistence;
	this->displacement = displacement;
	this->friendlyFire = friendlyFire;

}

// Called when the game starts or when spawned
void ADamager::BeginPlay()
{
	Super::BeginPlay();

	//set overlap function
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ADamager::OnOverlapBegin);
	if (damagerOwner != nullptr)
		damage = Cast<AEnemyClass>(damagerOwner)->dmgProjectile;
}

// Called every frame
void ADamager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ADamager::OnOverlapBegin);

	//Displace thing based on displacement
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Displacement: %f"), displacement.Z));	//for debug
	if (displacement != FVector(0.f, 0.f, 0.f))
		AddActorLocalOffset(displacement * DeltaTime * 35.f, true, &tickHit, ETeleportType::None);		//Since DeltaTime is very long at beginning of game (while launching,) shots existing at game start will jump forward.  It appears they don't collide with everything on the way, either.
	//AddActorLocalOffset(displacement, true, &tickHit ,ETeleportType::None);		//HEY This needs DeltaTime!
	
	//detect and respond to hit actors
	if (tickHit.Actor != nullptr)	
		HitObject(Cast<AActor>(tickHit.Actor));
	/*
	This is giving me some strange errors...
	It's spawning near the world origin
	It doesn't activate until I eject from the player character and lcick on it in the editor
	*/
}

void ADamager::HitObject(AActor *other)
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit something: %s"), *other->GetName() ));	//for debug

	//ensure it's a valid thing to hit
	if ((friendlyFire || other != damagerOwner))// && (other != this))
	{
		//This would be a cool place to spawn a particle system, sparks or something
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Valid hit")));	//for debug


		//handle reflection
		bool reflected = false;

		//if it has physics, add an impulse.
		if (other->GetRootComponent()->IsSimulatingPhysics())
		{
				//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Physics")));	//for debug
			TArray<UStaticMeshComponent*> Components;
			other->GetComponents<UStaticMeshComponent>(Components);
			for (int32 i = 0; i < Components.Num(); i++)		//found a tip here: https://answers.unrealengine.com/questions/127643/how-do-i-get-an-actors-static-mesh.html
			{
				UStaticMeshComponent* StaticMeshComponent = Components[i];
				StaticMeshComponent->AddImpulseAtLocation(this->GetVelocity(), this->GetActorLocation());

			}
		}
		//Make sure thing gets damaged if it's an IKillableEntity
		if (Cast<IIKillableEntity>(other) != nullptr)
		{
			//handle reflection if it's player character
			if (Cast<ASurvios_2019_01Character>(other) != nullptr)
				reflected = Cast<ASurvios_2019_01Character>(other)->GetRefl();
			else
				Cast<IIKillableEntity>(other)->TakeDamage(damage);
		}

		//reduce persistence, unless it got reflected
		if (persistence > 0 && !reflected)
		{
			persistence -= 1;
			if (persistence <= 0)
				DeathSequence();
		}
	}
	/*
	*/
}

void ADamager::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{

	HitObject(OtherActor);
}

bool ADamager::CheckFriendlyFire()
{
	return friendlyFire;
}

void ADamager::DeathSequence()
{
	Destroy();
}

void ADamager::AssignAttributes(AActor * dOwner, float setDamage, int newPersistence, FVector newDisplacement, bool fFire)
{
	this->damagerOwner = dOwner;
	this->damage = setDamage;
	this->persistence = newPersistence;
	this->displacement = newDisplacement;
	this->friendlyFire = fFire;
}

