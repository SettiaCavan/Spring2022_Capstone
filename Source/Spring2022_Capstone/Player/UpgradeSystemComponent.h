// Created by Spring2022_Capstone team

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Spring2022_Capstone/UI/UpgradeMenu/UpgradeMenuWidget.h"
#include "Spring2022_Capstone/Weapon/WeaponBase.h"
#include "UpgradeSystemComponent.generated.h"

UENUM()
enum EUpgradeType
{
	None,
	MaxChargeAmount,
	WeaponDamage,
	ChargeCooldown,
	MaxHealth,
	MovementSpeed,
	GrappleCooldown,
	UnlockDoubleJump,
	// ToDo: Ability Cooldown
	// ToDo: Grapple Range
	// ToDo: Dash Distance
	// ToDo: Crit Hit Multiplier
};

USTRUCT()
struct FUpgradeChoice
{
	GENERATED_BODY()
	FUpgradeChoice()
	{
		TypeOfUpgrade = EUpgradeType::None;
		UpgradeValue = 0;
	}

	FUpgradeChoice(const EUpgradeType Type, const float Value)
	{
		TypeOfUpgrade = Type;
		UpgradeValue = Value;
	}
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EUpgradeType> TypeOfUpgrade;

	UPROPERTY(EditAnywhere)
	float UpgradeValue;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Within=(PlayerCharacter))
class SPRING2022_CAPSTONE_API UUpgradeSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUpgradeSystemComponent();
	
	// Widget Blue Print used to create instance for Player's upgrade menu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUpgradeMenuWidget> UpgradeMenuWidgetBP;

	UPROPERTY()
	UUpgradeMenuWidget* UpgradeMenuWidgetInstance;
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	APlayerCharacter* PlayerToUpgrade;
	
	UPROPERTY()
	APlayerController* PlayerController;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

// Weapon Upgrades
	
	/**
	 * @brief Increases given weapon's MaxChargeAmount by Amount.
	 * @param WeaponToUpgrade  WeaponBase child to be changed.
	 * @param Amount Amount added to give weapon's MaxChargeAmount.
	 */
	UFUNCTION(BlueprintCallable)
		void IncreaseMaxChargeAmount(AWeaponBase* WeaponToUpgrade, float Amount);

	/**
	* @brief Increases the given AWeaponBase's ShotDamage.
	* @param Amount Value added to WeaponToUpgrade's ShotDamage.
	*/
	UFUNCTION(BlueprintCallable)
		void IncreaseWeaponDamageByAmount(AWeaponBase* WeaponToUpgrade, float Amount);

	/**
	 * @brief Increases the given AWeaponBase's ChargeCooldownRate.
	 * @param Amount Value added to WeaponToUpgrade's ChargeCooldownRate.
	 */
	UFUNCTION(BlueprintCallable)
		void IncreaseChargeCooldownRate(AWeaponBase* WeaponToUpgrade, float Amount);

// Health Upgrades

	/**
	 * @brief Increases the given character's max health by a float amount.
	 * @param Amount Amount to be added to Character's Max Health.
	 */
	UFUNCTION(BlueprintCallable)
		void IncreaseMaxHealthByAmount(float Amount);
	
	/**
	 * @brief Increases the character's max health by a percentage.
	 * @param PercentageAmount Percentage of health to be added to Character's Max Health.
	 */
	UFUNCTION(BlueprintCallable)
		void IncreaseMaxHealthByPercentage(float PercentageAmount);

// Movement Upgrades

	/**
	 * @brief Increases the character's movement speed by Amount.
	 * @param Amount Value added to the Character's current move speed.
	 */
	UFUNCTION(BlueprintCallable)
		void IncreaseMovementSpeedByAmount(int Amount);
	
	/**
	 * @brief Sets the Player's max jump amount to 2.
	 */
	UFUNCTION(BlueprintCallable)
		void UnlockDoubleJump();

	/**
	 * @brief Decrease the Player's grapple cooldown by Amount.
	 * @param Seconds Value decreased from AWeaponBase's ChargeCooldownRate.
	 */
	UFUNCTION(BlueprintCallable)
		void DecreaseGrappleCooldownBySeconds(float Seconds);

// UI

	UFUNCTION(BlueprintCallable)
	void OpenUpgradeMenu();

	UFUNCTION(BlueprintCallable)
	void CloseUpgradeMenu();

	UPROPERTY(BlueprintReadOnly)
	bool bIsMenuOpen;

// Upgrade Selection Functionality
	
	// Available upgrades for the player. ToDo: Remove EditAnywhere specifier.
	UPROPERTY(VisibleAnywhere, Category = "AvailableUpgrades")
	TArray<FUpgradeChoice> UpgradeChoices =
		{
			// Health
			FUpgradeChoice(EUpgradeType::MaxHealth, 5),
			FUpgradeChoice(EUpgradeType::MaxHealth, 5),
			FUpgradeChoice(EUpgradeType::MaxHealth, 5),
			FUpgradeChoice(EUpgradeType::MaxHealth, 5),
			FUpgradeChoice(EUpgradeType::MaxHealth, 5),
			// Weapon Damage (ToDo: Specific Weapon)
			FUpgradeChoice(EUpgradeType::WeaponDamage, 5),
			FUpgradeChoice(EUpgradeType::WeaponDamage, 5),
			FUpgradeChoice(EUpgradeType::WeaponDamage, 5),
			FUpgradeChoice(EUpgradeType::WeaponDamage, 5),
			FUpgradeChoice(EUpgradeType::WeaponDamage, 5),
			// Move Speed
			FUpgradeChoice(EUpgradeType::MovementSpeed, 5),
			FUpgradeChoice(EUpgradeType::MovementSpeed, 5),
			FUpgradeChoice(EUpgradeType::MovementSpeed, 5),
			// Double Jump
			FUpgradeChoice(EUpgradeType::UnlockDoubleJump, NULL),
			// Max Weapon Charge
			FUpgradeChoice(EUpgradeType::ChargeCooldown, 5),
			FUpgradeChoice(EUpgradeType::ChargeCooldown, 5),
			FUpgradeChoice(EUpgradeType::ChargeCooldown, 5),
			// Weapon Cooldown
			FUpgradeChoice(EUpgradeType::ChargeCooldown, 5),
			FUpgradeChoice(EUpgradeType::ChargeCooldown, 5),
			FUpgradeChoice(EUpgradeType::ChargeCooldown, 5),
			// ToDo: Ability Cooldown
			// ToDo: Grapple Range
			// ToDo: Dash Distance
			// ToDo: Crit Hit Multiplier
		};
	
};
