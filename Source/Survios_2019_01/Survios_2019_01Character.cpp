// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Survios_2019_01Character.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"	//Not to be confused with "Engine.h", which comes with a MILLION things
#include "Runtime/Engine/Public/EngineGlobals.h"
#include "GameFramework/PlayerController.h"
#include "EnemyClass.h"
#include "Engine/Console.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Damager.h"

//////////////////////////////////////////////////////////////////////////
// ASurvios_2019_01Character

ASurvios_2019_01Character::ASurvios_2019_01Character()
{
	//set input mode
	iMode = InputMode::FullControl;
	bCamAutoRotateX = true;

	//Set dampener for when jump button is released:
	hatZVelocity = 800.f;

	//And jump height for when you bounce on enemies
	bounceZVelocity = GetCharacterMovement()->JumpZVelocity * 0.75;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	//Collision for capsule set in BeginPlay

	maxHealth = 100.f;
	SetHealth(maxHealth);
	invincibilityTime = 2.2f;
	respawnTime = 3.f;

	//Abilities: Reflector Values
	refl = false;
	reflDuration = 0.8f;
	reflCooldown = 2.f;
	reflEnable = true;

	//score
	score = 0.f;
	combo = 0;
	pointsPerKill = 10.f;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	TurnRateMultiplier = 1.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	maxRunSpeed = 840;
	originalRunSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	//When character falls or rises, how should that affect camera?
	bCamAutoRotateY = true;
	fMaxFallCameraAngle = 70;
	fMaxFallCameraSpeed = 1600;
	fMinFallCameraAngle = -50;
	fMinFallCameraSpeed = -4000;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASurvios_2019_01Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASurvios_2019_01Character::JumpInput);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASurvios_2019_01Character::StopJumpingInput);

	//Abilities:
	PlayerInputComponent->BindAction("Reflector", IE_Released, this, &ASurvios_2019_01Character::StartReflInput);
	

	PlayerInputComponent->BindAxis("MoveForward", this, &ASurvios_2019_01Character::MoveForwardInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASurvios_2019_01Character::MoveRightInput);

	//Toggle camera autorotation
	PlayerInputComponent->BindAction("CameraMode", IE_Pressed, this, &ASurvios_2019_01Character::CameraModeInput);
	PlayerInputComponent->BindAction("PauseGame", IE_Pressed, this, &ASurvios_2019_01Character::SetGamePausedInput) .bExecuteWhenPaused = true;
	
	// We have 1 version of the rotation binding
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("TurnRate", this, &ASurvios_2019_01Character::TurnAtRateInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASurvios_2019_01Character::LookUpAtRateInput);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ASurvios_2019_01Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ASurvios_2019_01Character::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ASurvios_2019_01Character::OnResetVR);

	//Enable hit events for the capsule
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ASurvios_2019_01Character::OnCompHit);
}

void ASurvios_2019_01Character::EndInvincibility()
{
	invincible = false;
}

void ASurvios_2019_01Character::BeginPlay()
{
	Super::BeginPlay();	//This should be uncommented if this works properly

	SetHealth(maxHealth);

	//save camera's initial angle
	fCamInitialPitch = FollowCamera->GetComponentRotation().Pitch;

	//Enable hit events for the capsule
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ASurvios_2019_01Character::OnCompHit);
	//set overlap function, which calls OnComponentHit
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ASurvios_2019_01Character::OnOverlapBegin);
}

void ASurvios_2019_01Character::Tick(float DeltaTime)	//Note: The BP version of tick doesn't override this
{
	Super::Tick(DeltaTime);   

	//this may be better served by a timer, not totally sure
	//reset combo when player lands
	if (!GetCharacterMovement()->IsFalling())
		combo = 0;
	//set camera pitch based on rising/falling
	if (bCamAutoRotateY)
		FollowCamera->SetWorldRotation(SetCameraYRotation(this->GetVelocity().Z, FollowCamera->GetComponentRotation()));

}

bool ASurvios_2019_01Character::CameraMode()
{
	return bCamAutoRotateX;
}

bool ASurvios_2019_01Character::CameraMode(bool mode)
{
	//sets desired camera mode
	bCamAutoRotateX = mode;
	return CameraMode();
}

FRotator ASurvios_2019_01Character::SetCameraYRotation(float zVelocity, FRotator camAngle)
{
	//how much should the original angle be modified by?
	//float angleMod = (0.02142857142857142857142857142857 * zVelocity);// +35.714285714285714285714285714286;
		//Formula is derived from a linear function with the highest and lowest desired angles as points.
	//If it's above or below the max/min angles, use the max/min.
		//but it should not be hardcoded, let the designer set these variables.
	//we're using zvelocity as x, and resulting angle mod as y.    y = mx + b form.  m is a calculation of slope, b is y - mx (derived with algebra)
	float angleMod = (((fMaxFallCameraAngle - fMinFallCameraAngle) / (fMaxFallCameraSpeed - fMinFallCameraSpeed)) * zVelocity);// +(fMaxFallCameraAngle - (((fMaxFallCameraAngle - fMinFallCameraAngle) / (fMaxFallCameraSpeed - fMinFallCameraSpeed)) * zVelocity));
	if (angleMod > fMaxFallCameraAngle)
		return FRotator(camAngle.Roll, fCamInitialPitch + fMaxFallCameraAngle, camAngle.Yaw);
	else if (angleMod < fMinFallCameraAngle)
		return FRotator(camAngle.Roll, fCamInitialPitch + fMinFallCameraAngle, camAngle.Yaw);
	else
	{
		//return FRotator(camAngle.Roll, fCamInitialPitch + angleMod, camAngle.Yaw);
		//return FRotator(camAngle.Roll, camAngle.Pitch,camAngle.Yaw);
		return FRotator(fCamInitialPitch + angleMod, camAngle.Yaw, camAngle.Roll);
	}
}


void ASurvios_2019_01Character::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ASurvios_2019_01Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		JumpInput();
}

void ASurvios_2019_01Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumpingInput();
}

//Handle input for turning camera
void ASurvios_2019_01Character::TurnAtRateInput(float Rate)
{
	//If autorotate is off, or if there's significant input on X axis camera control, use manual rotation
	if (!bCamAutoRotateX || (Rate > 0.05f || Rate < -0.05f))
	{
		TurnAtRate(Rate);
	}
	else   //Auto-rotation based on L-Stick
	{
		TurnAtRate((1.5 - cos(LStickY) + (0.25*(-1-LStickY))) * 2 * (LStickX * 0.5));	//reuse & improvement of an algorithm developed for a previous project (FireWood)
	}

}

void ASurvios_2019_01Character::LookUpAtRateInput(float Rate)
{
	//Coming Soon!
	//LookUpAtRate(Rate);
}

void ASurvios_2019_01Character::CameraModeInput()
{
	CameraMode(!bCamAutoRotateX);
}

void ASurvios_2019_01Character::SetGamePausedInput()
{
	
	SetGamePaused(!GetWorld()->IsPaused());
}

void ASurvios_2019_01Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (iMode != InputMode::NoControl)
		AddControllerYawInput(Rate * BaseTurnRate * TurnRateMultiplier* GetWorld()->GetDeltaSeconds());
}

void ASurvios_2019_01Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (iMode != InputMode::NoControl)
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASurvios_2019_01Character::CapJumping()
{
	//Reduce Z-velocity to a set  level, so that player won't keep flyign upward when they simply tap the jump button
	FVector hatVelocity = GetVelocity();
	hatVelocity.Z = hatZVelocity;
	GetCharacterMovement()->Velocity = hatVelocity;
}

void ASurvios_2019_01Character::StartReflInput()
{
	if (iMode == InputMode::FullControl || iMode == InputMode::NoJumping)
	{
		StartRefl();
	}
}

void ASurvios_2019_01Character::JumpInput()
{
	if (iMode == InputMode::FullControl)
	{
			Jump();
	}
}

void ASurvios_2019_01Character::StopJumpingInput()
{
	if (iMode == InputMode::FullControl)
	{
		//Check if we're still jumping upward, or if we've hit max jumps.
		if (JumpForceTimeRemaining > 0.f || (JumpCurrentCount == JumpMaxCount && GetCharacterMovement()->Velocity.Z > hatZVelocity))	//ISSUE: If we have more than one jump enabled, it won't properly cap previous jumps.
		{
			CapJumping();
		}

		//run parent function
		StopJumping();
	}
}

void ASurvios_2019_01Character::MoveForwardInput(float Value)
{
	//Save value for camera auto-rotate
	LStickY = Value;
	if ((Controller != NULL) && (Value != 0.0f) && ((iMode == InputMode::FullControl) || (iMode == InputMode::NoJumping)))
	{
		//If we're not at max run speed, increase the run speed
		if (GetCharacterMovement()->MaxWalkSpeed < maxRunSpeed)
			GetCharacterMovement()->MaxWalkSpeed += (FApp::GetDeltaTime() * 100 *((1 + (GetCharacterMovement()->MaxWalkSpeed / originalRunSpeed))));
													//Detach from framerate,		Make rate of accel. increase as we go faster
		//Move the character
		MoveForward(Value);
	}
	else
		GetCharacterMovement()->MaxWalkSpeed = originalRunSpeed;
}

void ASurvios_2019_01Character::MoveRightInput(float Value)
{
	//Save value for cam auto-rotate
	LStickX = Value;
	if ((Controller != NULL) && (Value != 0.0f) && ((iMode == InputMode::FullControl) || (iMode == InputMode::NoJumping)))
	{
		MoveRight(Value);
	}
}

void ASurvios_2019_01Character::MoveForward(float Value)
{
	
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Value);
}

void ASurvios_2019_01Character::MoveRight(float Value)
{
	
	// find out which way is right
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	
	// get right vector 
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	// add movement in that direction
	AddMovementInput(Direction, Value);
	
}

void ASurvios_2019_01Character::SetGamePaused(bool pauseMode)
{ 
	GetWorld()->GetFirstPlayerController()->SetPause(pauseMode);
	/*
	if (pauseMode)
	{
		//Not implemented: Create widget
		//Helpful Guide: https://answers.unrealengine.com/questions/470481/create-widget-in-pure-c.html
		//Or maybe this: https://forums.unrealengine.com/development-discussion/c-gameplay-programming/42318-opening-a-widget-blueprint-from-c-code
	}
	else
		//Destroy Widget, assuming it's not null
	*/
}

//treat overlaps like hits
void ASurvios_2019_01Character::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{

	OnCompHit(OverlappedComp,OtherActor, OtherComp, FVector(0.f,0.f, 0.f), SweepResult);
}

void ASurvios_2019_01Character::OnCompHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	
	//for debug: What actor did we just touch
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("I Hit: %s"), *OtherActor->GetName()));

	//Start by checking if it's a character
	if (OtherActor->IsA(ACharacter::StaticClass()))
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("It's a character, too.")));
		//Handle character hit
		CharacterHit(OtherActor);
	}
	//Handle damagers
	else if (Cast<ADamager>(OtherActor) != nullptr)
	{
		DamagerHit(OtherActor);
	}
}

//If we overlap, treat it like a hit



void ASurvios_2019_01Character::CharacterHit(AActor *actor)
{
	//take damage from enemies, but not if they're invincible (which probably means we just hit them)
	AEnemyClass *enemy = Cast<AEnemyClass>(actor);
	if ((enemy != nullptr )&& !(enemy->invincible))
	{
		TakeDamage(enemy->dmgContact);
		//TakeDamage(20);
	}

}

void ASurvios_2019_01Character::DamagerHit(AActor *actor)
{
	ADamager *dam = Cast<ADamager>(actor);
	//take damage

	if (dam != nullptr)
	{

		if (dam->damagerOwner == this && !dam->CheckFriendlyFire())
			return;
		else if (refl )	//second condition: ensure it's not already been reflected.  Else it could reflect twice, no net effect
		{

			//if reflector is enabled, take over the object (well, only if it has displacement (projectile damagers)

			if (dam->displacement != FVector(0.f, 0.f, 0.f))
			{
				if (dam->persistence >= 0)
					dam->persistence += 1;
				dam->damagerOwner = this;

				dam->displacement *= -1.f;
			}
		}
		else
		{

			TakeDamage(dam->damage);
			dam->HitObject(this);
		}
	}
}

void ASurvios_2019_01Character::EnemyBounce()
{
	//add points for successful bounce
	AddPointsForJump();
	//Reduce Z-velocity to a set  level, so that player won't keep flyign upward when they simply tap the jump button
	FVector bounceVelocity = GetVelocity();
	bounceVelocity.Z = bounceZVelocity;
	GetCharacterMovement()->Velocity = bounceVelocity;
}

float ASurvios_2019_01Character::AddPoints(float points)
{
	score += points;
	if (score < 0.f)
		score = 0.f;
	return score;
}

void ASurvios_2019_01Character::AddPointsForJump()
{

	if (combo < 1)
		AddPoints(pointsPerKill * 1.f);
	else if (combo < 3)
		AddPoints(pointsPerKill * 2.f);
	else 
		AddPoints(pointsPerKill * 3.f);
	combo++;
}

void ASurvios_2019_01Character::TakeDamage(float damage)
{
	//invincibility and reflector
	if (invincible || refl)
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
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Got Damaged: %f"), damage));
		invincible = true;
		GetWorldTimerManager().SetTimer(invincibilityTimer, this, &ASurvios_2019_01Character::EndInvincibility,invincibilityTime,false, -1.f);
	}

}

void ASurvios_2019_01Character::Respawn()
{
	//reset health and position
	SetHealth(maxHealth);
	iMode = InputMode::FullControl;
	invincible = false;
	if (spawnPoint == nullptr)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Spawn Point Not Set!")));
		return;
	}
	SetActorTransform(spawnPoint->GetTransform());
	//Give player control
}

void ASurvios_2019_01Character::DeathSequence()
{
	iMode = InputMode::CameraOnly;
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("DeathSequenceRuns")));

	invincible = true;	//prevent this timer from restarting when we're damaged by something after death			//If we're getting hit by lasers after dead, this sequence would keep running , so the timer would never make it all the way thru to Respawn
	GetWorldTimerManager().SetTimer(respawnTimer, this, &ASurvios_2019_01Character::Respawn, respawnTime,false, -1.f);	

}

void ASurvios_2019_01Character::SetHealth(float newHealth)
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

void ASurvios_2019_01Character::StartRefl()
{
	if (!refl && reflEnable)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("StartRefl")));
		refl = true;
		reflEnable = false;
		GetWorldTimerManager().SetTimer(reflTimer, this, &ASurvios_2019_01Character::EndRefl, reflDuration, false, -1.f);
	}
}

void ASurvios_2019_01Character::EndRefl()
{
	refl = false;
	GetWorldTimerManager().SetTimer(reflTimer, this, &ASurvios_2019_01Character::ResetRefl, reflCooldown, false, -1.f);
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ReflEnd")));

	
}

void ASurvios_2019_01Character::ResetRefl()
{
	reflEnable = true;
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ReflReady")));

}
