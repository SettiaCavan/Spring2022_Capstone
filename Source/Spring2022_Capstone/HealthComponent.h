// Created by Spring2022_Capstone team

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPRING2022_CAPSTONE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UUpgradeSystemComponent;

public:
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	UFUNCTION()
	void SetHealth(float NewHealth);
	UFUNCTION()
	float GetHealth();
	UFUNCTION()
	float GetMaxHealth();
	UFUNCTION()
	void SetMaxHealth(float value);

private:
	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth;
	UPROPERTY(EditAnywhere, Category = "Health")
	float HealthPoints;
};
