// Fill out your copyright notice in the Description page of Project Settings.


#include "AAxe.h"
#include <Components/StaticMeshComponent.h>
#include <Components/BoxComponent.h>
#include <Kismet/GameplayStatics.h>
#include <DrawDebugHelpers.h>
#include <../Plugins/Runtime/ApexDestruction/Source/ApexDestruction/Public/DestructibleComponent.h>
#include <Engine/Engine.h>
#include <Particles/ParticleSystemComponent.h>

// Sets default values
AAAxe::AAAxe()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMesh"));
	RootComponent = StaticMesh;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("BoxCollision"));
	BoxComponent->SetupAttachment(RootComponent);

	ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(FName("ParticleSystem"));
	ParticleSystem->SetupAttachment(RootComponent);

	MovSpeed = 20000.0f;
	RotSpeed = 20.0f;
	RecoverSpeed = 1.0f;
	IsRotationActive = false;
	IsRecoverActive = false;
	Direction = 1;
	time = 0.0f;

}

void AAAxe::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void AAAxe::BeginPlay()
{
	Super::BeginPlay();	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AAAxe::OnAxeCollides);
}

class USceneComponent* AAAxe::GetHand() const
{
	return Hand;
}

void AAAxe::SetHand(class USceneComponent* val,FString handType)
{
	Hand = val;
	HandType = handType;
}

void AAAxe::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	IsRotationActive = false;
	ParticleSystem->Deactivate();
}

void AAAxe::OnAxeCollides(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor != nullptr && OtherComp->GetCollisionObjectType() != ECC_Destructible)
	{		
		StaticMesh->SetSimulatePhysics(false);
		IsRotationActive = false;
		ParticleSystem->Deactivate();
	}
}

void AAAxe::ThrowAxe()
{
	APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	FVector ForwardVector = Camera->GetActorForwardVector();
	FRotator Rotator = ForwardVector.Rotation();

	SetActorRotation(Rotator);

	ParticleSystem->Activate(true);
	StaticMesh->SetSimulatePhysics(true);
	IsRotationActive = true;
	StaticMesh->AddImpulse(ForwardVector * MovSpeed);
	Direction = 1;
}

void AAAxe::RecoverAxe()
{
	StaticMesh->SetSimulatePhysics(false);
	ParticleSystem->Activate(true);
	IsRotationActive = true;
	Direction = -1;
	time = 0.0f;

	UWorld* World = GetWorld();
	InitialAxePos = GetActorLocation();	
	if (Hand)
	{
		FinalAxePos = Hand->GetComponentLocation();
	}
	else
	{
		AActor* Player = UGameplayStatics::GetPlayerPawn(World, 0);
		FinalAxePos = Player->GetActorLocation();		
	}

	CalculateMiddlePoint(InitialAxePos, FinalAxePos);
	IsRecoverActive = true;
}

void AAAxe::CalculateMiddlePoint(const FVector& InitialPos, const FVector& FinalPos)
{
	FVector LocalDirection = InitialPos - FinalPos;
	FVector RightVector;
	if (HandType == "Right")
	{
		UE_LOG(LogTemp, Warning, TEXT("Right"));
		RightVector = FVector::CrossProduct(FVector::UpVector, LocalDirection).GetSafeNormal();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Left"));
		RightVector = FVector::CrossProduct(LocalDirection, FVector::UpVector).GetSafeNormal();
	}

	MiddleAxePos = FinalAxePos + LocalDirection * 0.2f + RightVector * 400.0f;
}

FVector AAAxe::QuadraticBezierCurve(float t,const FVector& InitialPos,const FVector& FinalPos,const FVector& MiddlePoint)
{
	FVector NewPos;
	NewPos = (1.0f - t) * (1.0f - t) * (InitialPos)+2 * t * (1 - t) * (MiddlePoint)+t * t * (FinalPos);
	return NewPos;
}

// Called every frame
void AAAxe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsRotationActive)
	{
		AddActorLocalRotation(FRotator(Direction * RotSpeed * DeltaTime, 0.0f, 0.0f));
	}

	if (IsRecoverActive)
	{
		if (time < 1.0f)
		{
			SetActorEnableCollision(false);
			FVector NewPos = QuadraticBezierCurve(time, InitialAxePos, FinalAxePos, MiddleAxePos);
			bool result = SetActorLocation(NewPos, true);
			time += DeltaTime * RecoverSpeed;
			if (!result)
			{
				IsRecoverActive = false;
				StaticMesh->SetSimulatePhysics(true);
				SetActorEnableCollision(true);
				K2_OnRecoverFinished(false);
			}
		}
		else
		{
			SetActorEnableCollision(true);
			IsRecoverActive = false;
			IsRotationActive = false;
			K2_OnRecoverFinished(true);
		}
	}

}

