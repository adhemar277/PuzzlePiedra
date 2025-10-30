// Copyright Epic Games, Inc. All Rights Reserved.

#include "PuzzlePiedraBlock.h"
#include "PuzzlePiedraBlockGrid.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstance.h"
#include "Kismet/GameplayStatics.h"

APuzzlePiedraBlock::APuzzlePiedraBlock()

{
	PrimaryActorTick.bCanEverTick = true;
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		ConstructorHelpers::FObjectFinderOptional<UMaterial> BaseMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlueMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> OrangeMaterial;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Puzzle/Meshes/PuzzleCube.PuzzleCube"))
			, BaseMaterial(TEXT("/Game/Puzzle/Meshes/BaseMaterial.BaseMaterial"))
			, BlueMaterial(TEXT("/Game/Puzzle/Meshes/BlueMaterial.BlueMaterial"))
			, OrangeMaterial(TEXT("/Game/Puzzle/Meshes/OrangeMaterial.OrangeMaterial"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Create static mesh component
	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh0"));
	BlockMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());
	BlockMesh->SetRelativeScale3D(FVector(1.f,1.f,0.25f));
	BlockMesh->SetRelativeLocation(FVector(0.f,0.f,25.f));
	BlockMesh->SetMaterial(0, ConstructorStatics.BlueMaterial.Get());
	BlockMesh->SetupAttachment(DummyRoot);
	BlockMesh->OnClicked.AddDynamic(this, &APuzzlePiedraBlock::BlockClicked);
	BlockMesh->OnInputTouchBegin.AddDynamic(this, &APuzzlePiedraBlock::OnFingerPressedBlock);

	// Save a pointer to the orange material
	BaseMaterial = ConstructorStatics.BaseMaterial.Get();
	BlueMaterial = ConstructorStatics.BlueMaterial.Get();
	OrangeMaterial = ConstructorStatics.OrangeMaterial.Get();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Piedra(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	MallaPiedra = Piedra.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Papel(TEXT("/Game/StarterContent/Shapes/Shape_Plane.Shape_Plane"));
	MallaPapel = Papel.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Tijera(TEXT("/Game/StarterContent/Shapes/Shape_Cone.Shape_Cone"));
	MallaTijera = Tijera.Object;

	//____ default skin materials (optional built-in fallbacks)
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PiedraMat(TEXT("/Game/Puzzle/Meshes/PiedraMat.PiedraMat"));
	if (PiedraMat.Succeeded()) PiedraMaterial = PiedraMat.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PapelMat(TEXT("/Game/Puzzle/Meshes/PapelMat.PapelMat"));
	if (PapelMat.Succeeded()) PapelMaterial = PapelMat.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> TijeraMat(TEXT("/Game/Puzzle/Meshes/TijeraMat.TijeraMat"));
	if (TijeraMat.Succeeded()) TijeraMaterial = TijeraMat.Object;
	//____

	//____ default sounds (optional fallbacks)
	static ConstructorHelpers::FObjectFinder<USoundBase> SWin(TEXT("/Game/Puzzle/Audio/WinCue.WinCue"));
	if (SWin.Succeeded()) SoundWin = SWin.Object;
	static ConstructorHelpers::FObjectFinder<USoundBase> SLose(TEXT("/Game/Puzzle/Audio/LoseCue.LoseCue"));
	if (SLose.Succeeded()) SoundLose = SLose.Object;
	static ConstructorHelpers::FObjectFinder<USoundBase> STie(TEXT("/Game/Puzzle/Audio/TieCue.TieCue"));
	if (STie.Succeeded()) SoundTie = STie.Object;
	static ConstructorHelpers::FObjectFinder<USoundBase> SLaunch(TEXT("/Game/Puzzle/Audio/LaunchCue.LaunchCue"));
	if (SLaunch.Succeeded()) SoundLaunch = SLaunch.Object;
	//____

	TipoActual = Tipo::Piedra;
}

void APuzzlePiedraBlock::BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked)
{
	HandleClicked();
}


void APuzzlePiedraBlock::OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	HandleClicked();
}

void APuzzlePiedraBlock::HandleClicked()
{
	// Check we are not already active
	if (estaRotando || !BlockMesh) return; // si esta rotando ignora el click

	
	EstaIniciado = true; //se marca el bloque al iniciar la ronda
	beginRoll(true);
}

void APuzzlePiedraBlock::Highlight(bool bOn)
{
	// Do not highlight if the block has already been activated.
	if (bIsActive)
	{
		return;
	}

	if (bOn)
	{
		BlockMesh->SetMaterial(0, BaseMaterial);
	}
	else
	{
		BlockMesh->SetMaterial(0, BlueMaterial);
	}
}

void APuzzlePiedraBlock::beginRoll(bool EnemigoRotando)
{
	if (OrangeMaterial) {
		BlockMesh->SetMaterial(0, OrangeMaterial);
	}
	if (!BlockMesh->IsSimulatingPhysics()) {
		BlockMesh->SetSimulatePhysics(true);
	}
	//____ play launch sound if available
	if (SoundLaunch)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundLaunch, GetActorLocation());
	}
	//____
	Lanzar();
	estaRotando = true;
	Contador =0.f;
	// Lanza al ENEMIGO en el mismo frame (sin recursión)
	if (EnemigoRotando && Enemigo && Enemigo != this && !Enemigo->estaRotando && Enemigo->BlockMesh) {
		Enemigo->EstaIniciado = false;
		if (!Enemigo->BlockMesh->IsSimulatingPhysics()) {
			Enemigo->BlockMesh->SetSimulatePhysics(true);
		}
		if (Enemigo->OrangeMaterial) {
			Enemigo->BlockMesh->SetMaterial(0, Enemigo->OrangeMaterial);
		}
		Enemigo->Lanzar();
		Enemigo->estaRotando = true;
		Enemigo->Contador =0.f;
	}
}

void APuzzlePiedraBlock::Lanzar()
{
	BlockMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	BlockMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	//hacemos que no concidere la masa
	BlockMesh->AddImpulse(FVector(0.f,0.f, Fuerza), NAME_None, true);
	const FVector Torque = FVector(FMath::RandRange(-500.f,500.f), FMath::RandRange(-500.f,500.f), FMath::RandRange(-500.f,500.f));
	BlockMesh->AddTorqueInDegrees(Torque, NAME_None, false);
}


FString APuzzlePiedraBlock::TipoText(Tipo T)
{
	switch (T)
	{
	case Tipo::Piedra:
		return FString("Piedra");
	case Tipo::Papel:
		return FString("Papel");
	case Tipo::Tijera:
		return FString("Tijera");
	default:
		return FString("Desconocido");
	}
}


void APuzzlePiedraBlock::EstableceTipo()
{
	int32 Rand = FMath::RandRange(0,2);
	TipoActual = static_cast<Tipo>(Rand);
}

void APuzzlePiedraBlock::AplicarMalla()
{
	if (MallaPiedra && MallaPapel && MallaTijera) {
		switch (TipoActual)
		{
		case Tipo::Piedra:
			BlockMesh->SetStaticMesh(MallaPiedra);
			//____ apply skin material if available
			if (PiedraMaterial) BlockMesh->SetMaterial(0, PiedraMaterial);
			break;
		case Tipo::Papel:
			BlockMesh->SetStaticMesh(MallaPapel);
			if (PapelMaterial) BlockMesh->SetMaterial(0, PapelMaterial);
			break;
		case Tipo::Tijera:
			BlockMesh->SetStaticMesh(MallaTijera);
			if (TijeraMaterial) BlockMesh->SetMaterial(0, TijeraMaterial);
			break;
		default:
			break;
		}
		return;
	}
}
void APuzzlePiedraBlock::ActivarMetodos()
{
	EstableceTipo();
	AplicarMalla();
	estaRotando = false;
	if (EstaIniciado && Enemigo && !Enemigo->estaRotando) {
		Resultado();
		EstaIniciado = false;
	}
}

void APuzzlePiedraBlock::Resultado()
{
	if (!Enemigo)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] No tiene Enemigo asignado."), *GetName());
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1,2.f, FColor::Red, TEXT("Asigna 'Enemigo' en el editor."));
		return;
	}

	// Si el enemigo no tiene tipo válido aún, dale uno y aplícale visual 
	if (Enemigo->TipoActual != Tipo::Piedra && Enemigo->TipoActual != Tipo::Papel && Enemigo->TipoActual != Tipo::Tijera)
	{
		Enemigo->EstableceTipo();
		Enemigo->AplicarMalla();
	}

	//2) Ambos listos: resuelve
	const Tipo A = TipoActual;
	const Tipo B = Enemigo->TipoActual;

	FString Resultado;
	if (A == B)
	{
		Resultado = TEXT("Empate");
	}
	else if ((A == Tipo::Piedra && B == Tipo::Tijera) || (A == Tipo::Papel && B == Tipo::Piedra) || (A == Tipo::Tijera && B == Tipo::Papel))
	{
		Resultado = TEXT("Gana JUGADOR");
	}
	else
	{
		Resultado = TEXT("Gana ENEMIGO");
	}

	const FString Msg = FString::Printf(TEXT("[%s] %s (%s) vs (%s) [%s]"),
		*GetName(),
		*Resultado,
		*TipoText(A),
		*TipoText(B),
		*Enemigo->GetName());

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);

	if (GEngine)
	{
		const FColor C = (Resultado == TEXT("Gana JUGADOR")) ? FColor::Green :
			(Resultado == TEXT("Empate")) ? FColor::Yellow :
			FColor::Red;
		GEngine->AddOnScreenDebugMessage(-1,10.f, C, Msg);
	}

	//____ play result sounds
	if (Resultado == TEXT("Gana JUGADOR") && SoundWin)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundWin, GetActorLocation());
	}
	else if (Resultado == TEXT("Gana ENEMIGO") && SoundLose)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundLose, GetActorLocation());
	}
	else if (Resultado == TEXT("Empate") && SoundTie)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundTie, GetActorLocation());
	}
	//____

	// (Opcional) si usas bIsActive para bloquear clicks, libéralo aquí:
	bIsActive = false;

	// (Opcional) si usas bIsInitiator para que sólo uno anuncie, puedes limpiar aquí:
	EstaIniciado = false;
}



void APuzzlePiedraBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (estaRotando && BlockMesh && BlockMesh->IsSimulatingPhysics())
	{
		const FVector LinV = BlockMesh->GetPhysicsLinearVelocity();
		const FVector AngV = BlockMesh->GetPhysicsAngularVelocityInDegrees();

		// Umbrales de "movimiento lento"
		const bool bSlow = (LinV.Size() <3.f) && (AngV.Size() <5.f);
		Contador = bSlow ? (Contador + DeltaTime) :0.f;

		// Requiere0.3s continuos de calma para considerarse "asentado"
		if (Contador >0.3f)
		{
			estaRotando = false;
			Contador =0.f;
			ActivarMetodos();//activa llama a los3 metodos 
		}
	}
}