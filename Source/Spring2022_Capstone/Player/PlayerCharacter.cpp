// Created by Spring2022_Capstone team

#include "PlayerCharacter.h"
#include "GrappleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GrappleState.h"
#include "Spring2022_Capstone/Weapon/WeaponBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Spring2022_Capstone/HealthComponent.h"
#include "Kismet/KismetMathLibrary.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	Camera->bUsePawnControlRotation = true;

	GrappleComponent = CreateDefaultSubobject<UGrappleComponent>(TEXT("Grapple"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	CrouchEyeOffset = FVector(0.f);
	CrouchSpeed = 12.f;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent *EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Dash);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Grapple);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Crouch);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Attack);
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Completed, this,
										   &APlayerCharacter::SwitchWeapon);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Sprint);
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController *PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<
				UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CharacterMappingContext, 0);
		}
	}
	Speed = GetCharacterMovement()->MaxWalkSpeed;
	UpdateHealthBar();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float CrouchInterpTime = FMath::Min(1.f, CrouchSpeed * DeltaTime);
	CrouchEyeOffset = (1.f - CrouchInterpTime) * CrouchEyeOffset;
}

void APlayerCharacter::Move(const FInputActionValue &Value)
{
	const FVector2D DirectionalValue = Value.Get<FVector2D>();
	if (GetController() && (DirectionalValue.X != 0.f || DirectionalValue.Y != 0.f))
	{
		GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? Speed * SprintMultiplier : Speed;
		AddMovementInput(GetActorForwardVector(), DirectionalValue.Y * 100);
		AddMovementInput(GetActorRightVector(), DirectionalValue.X * 100);
	}
}

void APlayerCharacter::Dash(const FInputActionValue &Value)
{
	const float CurrentTime = GetWorld()->GetRealTimeSeconds();

	if (bCanDash)
	{

		// If Player Double Taps the same direction
		if (CurrentTime - LastDashActionTappedTime < DoubleTapActivationDelay && Value.GetMagnitude() == PreviousDashDirection)
		{

			// Knock the actor up slightly to prevent ground collision
			LaunchCharacter(FVector(0, 0, 250), false, true); // Note: I like the feel of true Overrides but we can come back later.

			// Set Dash DirectionalValue to be used in DashDirectionLaunch
			DashDirectionalValue = Value.Get<FVector2D>();

			// After a tiny delay dash in desired direction
			GetWorld()->GetTimerManager().SetTimer(DashDirectionalMovementDelayTimerHandle, this, &APlayerCharacter::DashDirectionalLaunch, 0.065, false); // Note: This number will never change while running. 0.65 feels good.
		}
	}

	LastDashActionTappedTime = CurrentTime;
	PreviousDashDirection = Value.GetMagnitude();
}

void APlayerCharacter::DashDirectionalLaunch()
{
	const float PreDashSpeed = GetVelocity().Length();

	if (DashDirectionalValue.Y == 1)
		LaunchCharacter(GetActorForwardVector() * DashDistance, true, false);
	else if (DashDirectionalValue.Y == -1)
		LaunchCharacter(-GetActorForwardVector() * DashDistance, true, false);
	else if (DashDirectionalValue.X == -1)
		LaunchCharacter(-GetActorRightVector() * DashDistance, true, false);
	else if (DashDirectionalValue.X == 1)
		LaunchCharacter(GetActorRightVector() * DashDistance, true, false);

	// Handle velocity after dash
	FVector PostDashDirection = UKismetMathLibrary::Conv_RotatorToVector(GetCharacterMovement()->GetLastUpdateRotation());
	PostDashDirection *= PreDashSpeed;
	GetCharacterMovement()->Velocity = PostDashDirection;

	// Handle Dash Cooldown
	bCanDash = false;
	GetWorld()->GetTimerManager().SetTimer(DashCooldownTimerHandle, this, &APlayerCharacter::ResetDashCooldown, DashCooldownTime, false);

	LastDashActionTappedTime = 0;
}

void APlayerCharacter::ResetDashCooldown()
{
	bCanDash = true;
}

void APlayerCharacter::Look(const FInputActionValue &Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisValue.X * TurnRate * UGameplayStatics::GetWorldDeltaSeconds(this));
		AddControllerPitchInput(LookAxisValue.Y * TurnRate * UGameplayStatics::GetWorldDeltaSeconds(this));
	}
}

void APlayerCharacter::Sprint(const FInputActionValue &Value)
{
	bIsSprinting = Value.Get<bool>();
}

void APlayerCharacter::Crouch(const FInputActionValue &Value)
{
	if (Value.Get<bool>())
	{
		Super::Crouch();
	}
	else
	{
		UnCrouch();
	}
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (HalfHeightAdjust == 0.f)
	{
		return;
	}

	float StartBaseEyeHeight = BaseEyeHeight;
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CrouchEyeOffset.Z += StartBaseEyeHeight - BaseEyeHeight + HalfHeightAdjust;
	Camera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight), false);
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	float StartBaseEyeHeight = BaseEyeHeight;
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CrouchEyeOffset.Z += StartBaseEyeHeight - BaseEyeHeight - HalfHeightAdjust;
	Camera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight), false);
}

void APlayerCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo &OutResult)
{
	if (Camera)
	{
		Camera->GetCameraView(DeltaTime, OutResult);
		OutResult.Location += CrouchEyeOffset;
	}
}

void APlayerCharacter::Attack(const FInputActionValue &Value)
{
	if (bIsSprinting)
		return;
	ActiveWeapon->Shoot();
}

void APlayerCharacter::Grapple(const FInputActionValue &Value)
{
	if (!Value.Get<bool>())
	{
		GrappleComponent->CancelGrapple();
		return;
	}
	if (GrappleComponent->GrappleState != EGrappleState::ReadyToFire)
	{
		return;
	}
	FHitResult HitResult;
	FVector StartLocation = Camera->GetComponentLocation();
	FVector EndLocation = Camera->GetForwardVector() * GrappleComponent->GrappleRange + StartLocation;
	FCollisionQueryParams TraceParams;

	GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility);
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 5.f);
	FVector TargetLocation = EndLocation;
	if (AActor *HitActor = HitResult.GetActor())
	{
		TargetLocation = HitResult.ImpactPoint;
	}
	GrappleComponent->Fire(TargetLocation);
}

void APlayerCharacter::SwitchWeapon(const FInputActionValue &Value)
{
	ActiveWeapon = (ActiveWeapon == Weapon1) ? Weapon2 : Weapon1;
	if (OnWeaponSwitchedDelegate.IsBound())
	{
		OnWeaponSwitchedDelegate.Execute(ActiveWeapon);
	}
}

void APlayerCharacter::SetWeapon1(AWeaponBase *Weapon)
{
	Weapon1 = Weapon;
	ActiveWeapon = Weapon1;

	if (OnWeaponSwitchedDelegate.IsBound())
	{
		OnWeaponSwitchedDelegate.Execute(ActiveWeapon);
	}
}

void APlayerCharacter::SetWeapon2(AWeaponBase *Weapon)
{
	Weapon2 = Weapon;
	ActiveWeapon = Weapon2;

	if (OnWeaponSwitchedDelegate.IsBound())
	{
		OnWeaponSwitchedDelegate.Execute(ActiveWeapon);
	}
}

AWeaponBase *APlayerCharacter::GetWeapon1() const
{
	return Weapon1;
}

AWeaponBase *APlayerCharacter::GetWeapon2() const
{
	return Weapon2;
}

AWeaponBase* APlayerCharacter::GetActiveWeapon() const
{
	return ActiveWeapon;
}

void APlayerCharacter::TakeHit()
{
	if (HealthComponent)
	{
		HealthComponent->SetHealth(HealthComponent->GetHealth() - 5.0f);
		UpdateHealthBar();
	}
}

void APlayerCharacter::IncreaseMaxHealth(int Value)
{
	HealthComponent->SetMaxHealth(HealthComponent->GetMaxHealth() + Value);
	HealthComponent->SetHealth(HealthComponent->GetHealth() + Value);
	GEngine->AddOnScreenDebugMessage(0, 4.f, FColor::Red, FString::Printf(TEXT("Your new max health is: %f"), HealthComponent->GetMaxHealth()));
}

void APlayerCharacter::IncreaseMaxHealthPercentage(int Percentage)
{
	float HealthIncrease = HealthComponent->GetMaxHealth() * Percentage / 100;
	HealthComponent->SetMaxHealth(HealthComponent->GetMaxHealth() + HealthIncrease);
	HealthComponent->SetHealth(HealthComponent->GetHealth() + HealthIncrease);
	GEngine->AddOnScreenDebugMessage(0, 4.f, FColor::Red, FString::Printf(TEXT("Your new max health is: %f"), HealthComponent->GetMaxHealth()));
}

void APlayerCharacter::IncreaseMovementSpeed(int Value)
{
	Speed += Value;
	GEngine->AddOnScreenDebugMessage(0, 4.f, FColor::Red, FString::Printf(TEXT("Your new Movement Speed is: %f"), Speed));
}

void APlayerCharacter::IncreaseDamagePrimary(float Value)
{
	if (!Weapon1)
		return;
	Weapon1->SetDamage(Weapon1->GetDamage() + Value);
	GEngine->AddOnScreenDebugMessage(0, 4.f, FColor::Red, FString::Printf(TEXT("Your new Primary Weapon Damage is: %f"), Weapon1->GetDamage()));
}

void APlayerCharacter::IncreaseDamageSecondary(float Value)
{
	if (!Weapon2)
		return;
	Weapon2->SetDamage(Weapon2->GetDamage() + Value);
	GEngine->AddOnScreenDebugMessage(0, 4.f, FColor::Red, FString::Printf(TEXT("Your new Secondary Weapon Damage is: %f"), Weapon2->GetDamage()));
}

void APlayerCharacter::ToggleDoubleJump()
{
	JumpMaxCount = JumpMaxCount == 1 ? 2 : 1;
	GEngine->AddOnScreenDebugMessage(0, 4.f, FColor::Red, FString::Printf(TEXT("Your max jumps are: %i"), JumpMaxCount));
}

void APlayerCharacter::Heal(int Value)
{
	if (!HealthComponent)
		return;
	HealthComponent->SetHealth(HealthComponent->GetHealth() + Value);
	UpdateHealthBar();
}

void APlayerCharacter::HealByPercentage(int Percentage)
{
	if (!HealthComponent)
		return;
	HealthComponent->SetHealth(HealthComponent->GetHealth() + HealthComponent->GetMaxHealth() * Percentage / 100);
	UpdateHealthBar();
}

float APlayerCharacter::GetMaxHealth() const
{
	return HealthComponent->GetMaxHealth();
}

UGrappleComponent *APlayerCharacter::GetGrappleComponent()
{
	return GrappleComponent;
}

void APlayerCharacter::UpdateHealthBar()
{
	if (OnHealthChangedDelegate.IsBound())
	{
		OnHealthChangedDelegate.Execute(HealthComponent->GetHealth());
	}
}