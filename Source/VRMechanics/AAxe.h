// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AAxe.generated.h"


UCLASS()
class VRMECHANICS_API AAAxe : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAAxe();

protected:
	// Called when the game starts or when spawned  
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Mechanics")
	class UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Mechanics")
	class UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanics")
	float MovSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanics")
	float RotSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanics")
	float RecoverSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mechanics")
	bool IsRecoverActive;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mechanics")
	class UParticleSystemComponent* ParticleSystem;



protected:

	UFUNCTION()
	virtual void OnAxeCollides(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable,Category = "Mechanics")
	virtual void ThrowAxe();

	UFUNCTION(BlueprintCallable, Category = "Mechanics")
	virtual void RecoverAxe();

	UFUNCTION(BlueprintImplementableEvent, Category = "Mechanics")
	void K2_OnRecoverFinished(bool success);



private:
	bool IsRotationActive;

	int Direction;

	float time;

	class USceneComponent* Hand;
	FString HandType;

private:

	FVector QuadraticBezierCurve(float t, const FVector& InitialPos, const FVector& FinalPos, const FVector& MiddlePoint);

	void CalculateMiddlePoint(const FVector& InitialPos, const FVector& FinalPos);

	FVector InitialAxePos, MiddleAxePos, FinalAxePos;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void PostInitializeComponents() override;

	void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	UFUNCTION(BlueprintCallable)
	class USceneComponent* GetHand() const;

	UFUNCTION(BlueprintCallable)
	void SetHand(class USceneComponent* val, FString handType);

};
