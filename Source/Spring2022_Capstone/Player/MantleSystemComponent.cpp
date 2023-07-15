// Created by Spring2022_Capstone team


#include "MantleSystemComponent.h"
#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UMantleSystemComponent::UMantleSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UMantleSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<APlayerCharacter>(GetOwner());
	PlayerCharacterMovementComponent = Player->GetCharacterMovement();
	PlayerCapsuleComponent = Player->GetCapsuleComponent();

	SetTraceParams();

	// Setup Timeline
	FOnTimelineFloat TimelineCallback;
	FOnTimelineEventStatic TimelineFinishedCallback;

	// Length of Timeline is set from of MantleTimelineCurve; Length should match duration of CameraShake.
	if(MantleTimelineFloatCurve)
	{
		TimelineFinishedCallback.BindUFunction(this, FName(TEXT("TimelineFinishedCallback")));
		MantleTimeline.AddInterpFloat(MantleTimelineFloatCurve, TimelineCallback); 
		MantleTimeline.SetTimelineFinishedFunc(TimelineFinishedCallback);
	}
}

void UMantleSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	MantleTimeline.TickTimeline(DeltaTime); 
	
	if(bCanMantle && !MantleTimeline.IsPlaying())
	{
		UGameplayStatics::GetPlayerCameraManager(GetWorld(),0)->StartCameraShake(ClimbingCameraShake);
		MantleTimeline.PlayFromStart();
	}
	if(MantleTimeline.IsPlaying())
	{
		Player->SetActorLocation(FMath::Lerp(InitialPlayerPosition, TargetLocation, MantleTimeline.GetPlaybackPosition()));
	}
	
}

void UMantleSystemComponent::Mantle()
{

	/////////////////// Check for blocking wall /////////////////
	FHitResult BlockingWallHitResult;
	
	FVector BlockingWallCheckStartLocation = PlayerCharacterMovementComponent->GetActorLocation();
	BlockingWallCheckStartLocation.Z += CAPSULE_TRACE_ZAXIS_RAISE; // Raise capsule trace to avoid lower surfaces.

	FVector BlockingWallCheckEndLocation = BlockingWallCheckStartLocation + (GetOwner()->GetActorForwardVector() * CAPSULE_TRACE_REACH); 

	if(GetWorld()->SweepSingleByChannel(BlockingWallHitResult, BlockingWallCheckStartLocation, BlockingWallCheckEndLocation, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeCapsule(CAPSULE_TRACE_RADIUS, PlayerCapsuleComponent->GetScaledCapsuleHalfHeight()), TraceParams))
	{

		if(!PlayerCharacterMovementComponent->IsWalkable(BlockingWallHitResult))
		{
			// Climbable Object Hit
			InitialPoint = BlockingWallHitResult.ImpactPoint;
			InitialNormal = BlockingWallHitResult.ImpactNormal;
		}
	}
	else
	{
		// Didnt hit a wall.
		bCanMantle = false; 
		return; 
	}

	
	/////////////////// Trace downwards for surface /////////////////
	FHitResult SurfaceCheckHitResult;

	FVector SurfaceCheckStartLocation = FVector(InitialPoint.X, InitialPoint.Y, PlayerCharacterMovementComponent->GetActorLocation().Z - CAPSULE_TRACE_ZAXIS_RAISE) + InitialNormal * MANTLE_SURFACE_DEPTH; // Subtracting to account for raise above.
	SurfaceCheckStartLocation.Z += PlayerCapsuleComponent->GetScaledCapsuleHalfHeight() * 2;
	
	FVector SurfaceCheckEndLocation = FVector(InitialPoint.X, InitialPoint.Y, PlayerCharacterMovementComponent->GetActorLocation().Z - CAPSULE_TRACE_ZAXIS_RAISE) + InitialNormal * MANTLE_SURFACE_DEPTH; // Subtracting to account for raise above.
	
	if(GetWorld()->SweepSingleByChannel(SurfaceCheckHitResult, SurfaceCheckStartLocation, SurfaceCheckEndLocation, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(CAPSULE_TRACE_RADIUS), TraceParams))
	{

		if(SurfaceCheckHitResult.bBlockingHit && PlayerCharacterMovementComponent->IsWalkable(SurfaceCheckHitResult))
		{ 
			// Can climb up here.
			TargetLoc = SurfaceCheckHitResult.Location;
		}
		else
		{
			// Cannot mantle. // This gets called when there is a hit but it is now a walkable surface.
			bCanMantle = false; 
			TargetLocation = FVector(0, 0, 0); 
			return;
		}
	}
	else
	{
		// Object to tall.
		bCanMantle = false; 
		TargetLocation = FVector(0, 0, 0); 
		return;
	}
	
	/////////////////// Start Mantle (Handled in Tick) /////////////////
	PlayerCharacterMovementComponent->SetMovementMode(MOVE_None); // Stop player from moving while mantle-ing.

	TargetLocation = FVector(TargetLoc.X, TargetLoc.Y, TargetLoc.Z + 300);
	InitialPlayerPosition = Player->GetActorLocation();
	bCanMantle = true;
}


void UMantleSystemComponent::SetTraceParams()
{
	TraceParams.bTraceComplex = true;

	// Ignore Player and all it's components.
	TraceParams.AddIgnoredActor(GetOwner());
	// Warning. Anything that causes Component ownership change or destruction will invalidate array.
	TSet<UActorComponent*> ComponentsToIgnore = GetOwner()->GetComponents(); 
	for (UActorComponent* Component : ComponentsToIgnore)
		TraceParams.AddIgnoredComponent(Cast<UPrimitiveComponent>(Component));
	
	// ToDo: Ensure weapons are ignored when meshes added
}

void UMantleSystemComponent::TimelineFinishedCallback()
{
	PlayerCharacterMovementComponent->SetMovementMode(MOVE_Walking);
	MantleTimeline.Stop();
	bCanMantle = false;
}
