// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Bullet.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Spring Arm을 생성해서 Root에 붙인다.
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp-> SetupAttachment(RootComponent);
	//Spring Arm -> Target Arm Length : 400 
	SpringArmComp->TargetArmLength = 400;
	// 위치 (X=0.000000,Y=30.000000,Z=80.000000)
	SpringArmComp->SetRelativeLocation(FVector(0, 30, 80));

	// Camera를 생성해서 Spring Arm 에 붙인다.
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// Mesh에 3D 에셋을 로드해서 넣어준다.
	// 생성자 도우미를 이용해서 스켈레탈 메쉬를 로드한다.
	ConstructorHelpers::FObjectFinder<USkeletalMesh> tempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn'"));

	// 만약, 로드가 성공했다면
	if (tempMesh.Succeeded())
	{
		// 메쉬에 넣는다.
		GetMesh()->SetSkeletalMesh(tempMesh.Object);
		// 메쉬를 넣은 다음에 캐릭터의 위치 변경과 회전을 했다.
		// 위치 (X=0.000000,Y=0.000000,Z=-80.000000)
		// 회전 (Pitch=0.000000,Yaw=-90.000000,Roll=0.000000)
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -80), FRotator(0, -90, 0));

		bUseControllerRotationYaw = false;
		SpringArmComp->bUsePawnControlRotation = true;
		CameraComp->bUsePawnControlRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;

		JumpMaxCount = 2;
		GetCharacterMovement()->AirControl = 1;

		//권총 (BerettaPistol 의 정보)
		// (X=-25.304263,Y=40.376206,Z=130.000000)
		// (Pitch=0.000000,Yaw=-69.999999,Roll=0.000000)
		// (X=2.000000,Y=2.000000,Z=2.000000)
		// /Script/Engine.SkeletalMesh'/Game/Resource/GunBeretta/source/9mm_Hand_gun.9mm_Hand_gun'

		// 권총을 생성하고 에셋을 적용해서 플레이어에 배치하자
		HandGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BerettaPistol"));
		// 권총을 Mesh에 붙인다.
		HandGun->SetupAttachment(GetMesh(), TEXT("FirePosition"));
		HandGun->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		ConstructorHelpers::FObjectFinder<USkeletalMesh> tempHandGun(TEXT("/Script/Engine.SkeletalMesh'/Game/Resource/GunBeretta/source/9mm_Hand_gun.9mm_Hand_gun'"));

		if (tempHandGun.Succeeded())
		{
			HandGun->SetSkeletalMesh(tempHandGun.Object);
			HandGun->SetRelativeLocationAndRotation(FVector(-25.3f, 40.4f, 130.0f), FRotator(0, -70, 0));
			HandGun->SetRelativeScale3D(FVector(2.f));
		}

		// 스나이퍼 건을 생성해서 Mesh에 붙이기
		SniperGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CheyTacRifle"));
		SniperGun->SetupAttachment(GetMesh(), TEXT("ShootPosition"));
		SniperGun->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// 에셋도 로드해서 적용한다.
		ConstructorHelpers::FObjectFinder<USkeletalMesh> tempSniperGun(TEXT("/Script/Engine.SkeletalMesh'/Game/Resource/GunCheyTac/source/model.model'"));

		if (tempSniperGun.Succeeded())
		{
			SniperGun->SetSkeletalMesh(tempSniperGun.Object);
			SniperGun->SetRelativeLocation(FVector(-20.0f, 80.0f, 150.0f));
			SniperGun->SetRelativeScale3D(FVector(0.02f));
		}
	}
}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	// 시작할 때 두 개의 위젯을 생성한다.
	CrosshairUI = CreateWidget(GetWorld(), CrosshariUIfactory);
	SniperUI = CreateWidget(GetWorld(), SniperUIfactory);
	// 일반 조준 모드 CrosshairUI 화면에 표시
	CrosshairUI->AddToViewport();

	auto pc = Cast<APlayerController>(Controller);
	if (pc)
	{
		auto subsystem = ULocalPlayer::GetSubsystem <UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		if (subsystem)
		{
			subsystem->AddMappingContext(IMC_TPS, 0);
		}
	}

	// 권총(Hand Gun)으로 기본 설정
	ChangeToHandGun(FInputActionValue());
	
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PlayerMove();
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// 입력 함수들을 모두 연결

	auto PlayerInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (PlayerInput)
	{
		PlayerInput->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &ATPSPlayer::Turn);
		PlayerInput->BindAction(IA_LookUp, ETriggerEvent::Triggered, this, &ATPSPlayer::LookUp);
		PlayerInput->BindAction(IA_PlayerMove, ETriggerEvent::Triggered, this, &ATPSPlayer::Move);
		PlayerInput->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ATPSPlayer::InputJump);
		PlayerInput->BindAction(IA_Fire, ETriggerEvent::Started, this, &ATPSPlayer::InputFire);
		PlayerInput->BindAction(IA_HandGun, ETriggerEvent::Started, this, &ATPSPlayer::ChangeToHandGun);
		PlayerInput->BindAction(IA_SniperGun, ETriggerEvent::Started, this, &ATPSPlayer::ChangeToSniperGun);
		//스나이퍼 조준 모드
		PlayerInput->BindAction(IA_Sniper, ETriggerEvent::Started, this, &ATPSPlayer::SniperAim);
		PlayerInput->BindAction(IA_Sniper, ETriggerEvent::Completed, this, &ATPSPlayer::SniperAim);
	}
}

void ATPSPlayer::Turn(const FInputActionValue& inputValue)
{
	float value = inputValue.Get<float>();
	AddControllerYawInput(value);
}

void ATPSPlayer::LookUp(const FInputActionValue& inputValue)
{
	float value = inputValue.Get<float>();
	AddControllerPitchInput(value);
}

void ATPSPlayer::Move(const FInputActionValue& inputValue)
{
	FVector2D value = inputValue.Get<FVector2D>();
	// 상하 입력 처리
	Direction.X = value.X;
	// 좌우 입력 처리
	Direction.Y = value.Y;
}

void ATPSPlayer::InputJump(const FInputActionValue& inputValue)
{
	Jump();
}

void ATPSPlayer::PlayerMove()
{
	Direction = FTransform(GetControlRotation()).TransformVector(Direction);
	// 동속 운동 공식 : P(이동할 위치) = Po(현재 위치) + vt(속도*방향) * T(시간)
	//FVector P0 = GetActorLocation;
	//FVector VT = Direction * WalkSpeed * DeltaTime;
	//SetActorLocation(P);
	AddMovementInput(Direction);
	Direction = FVector::ZeroVector;
}

void ATPSPlayer::InputFire(const FInputActionValue& inputValue)
{
	// 총알을 생성해서 권총의 총구 위치에 배치한다. 
	if (bUsingHandGun * (true))
	{
		FTransform FirePosition = HandGun->GetSocketTransform(TEXT("FirePosition"));
		GetWorld()->SpawnActor<ABullet>(BulletFactory, FirePosition);
	}
	else if(bUsingSniperGun* (true))
 	{
		FTransform ShootPosition = SniperGun->GetSocketTransform(TEXT("ShootPosition"));
		GetWorld()->SpawnActor<ABullet>(BulletFactory, ShootPosition);

		//LineTrance 의 시작 위치
		FVector StartPosition = CameraComp->GetComponentLocation();
		//LineTrace의 종료 위치
		FVector EndPostion = CameraComp->GetComponentLocation() + CameraComp->GetForwardVector() * 100000;
		//LineTrace 의 충돌 정보를 담을 변수
		FHitResult HitInfo;
		//충돌 옵션 설정 변수
		FCollisionQueryParams Params;
		//자기 자신(플레이어)는 충돌에서 제외
		Params.AddIgnoredActor(this);
		//Channel 필터를 이용한 LineTrace 충돌 검출
		// 충돌 정보, 시작 위치, 종료 위치, 검출 채널, 충돌 옵션
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, StartPosition, EndPostion, ECC_Visibility, Params);

		// 충돌 처리 -> 총알 파편 효과 재생
		if (bHit)
		{
			// 총알 파편 효과 트랜스폼
			FTransform BulletTransform;
			// 부딪힌 위치 할당
			BulletTransform.SetLocation(HitInfo.ImpactPoint);
			// 총알 파편 효과 객체 생성
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletEffectFactory, BulletTransform);
		}

		// 부딪힌 물체의 컴포넌트에 물리가 적용되어 있으면 날려버린다.
		auto HitComp = HitInfo.GetComponent();
		// 1. 만약 컴포넌트에 물리가 적용되어 있다면
		if (HitComp && HitComp->IsSimulatingPhysics())
		{
			// 2. 조준한 방향이 필요
			FVector Dir = (EndPostion - StartPosition).GetSafeNormal();

			FVector Force = Dir * HitComp->GetMass() * 500000;
			// 3. 그 방향으로 날리고 싶다.
			HitComp->AddForceAtLocation(Force, HitInfo.ImpactPoint);
		}
	}

}

void ATPSPlayer::ChangeToHandGun(const FInputActionValue& inputValue)
{
	// 권총(Hand Gun) 사용 중 
	bUsingHandGun = true;
	SniperGun->SetVisibility(false);
	HandGun->SetVisibility(true);
	/*CrosshairUI->AddToViewport();
	SniperUI->RemoveFromParent();*/
	//CameraComp->FieldOfView = 90;

}

void ATPSPlayer::ChangeToSniperGun(const FInputActionValue& inputValue)
{
	// 소총(Sniper Gun) 사용
	bUsingHandGun = false;
	SniperGun->SetVisibility(true);
	HandGun->SetVisibility(false);
	/*SniperUI->AddToViewport();
	CrosshairUI->RemoveFromParent();*/
	//CameraComp->FieldOfView = 45;
}

void ATPSPlayer::SniperAim(const FInputActionValue& inputValue)
{
	// 스나이퍼 모드가 아닐 경우 처리하지 않는다.
	if (bUsingHandGun)
	{
		return;
	}
	// Pressed(Started) 입력 처리 
	if (bSniperAim == false)
	{
		// 스나이퍼 조준 모드 활성화
		bSniperAim = true;
		// 스나이퍼 조준 UI 등록
		SniperUI->AddToViewport();
		CameraComp->SetFieldOfView(45.0f);
		CrosshairUI->RemoveFromParent();
	}
	// Released(Completed) 입력 처리
	else
	{
		// 스나이퍼 조준 모드 비활성화
		bSniperAim = false;
		// 스나이퍼 조준 UI 화면모드 제거
		SniperUI->RemoveFromParent();
		CameraComp->SetFieldOfView(90.0f);
		CrosshairUI->AddToViewport();
	}
}

