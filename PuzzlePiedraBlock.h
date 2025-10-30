// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzlePiedraBlock.generated.h"

class UMaterialInstance;
class USoundBase;

UENUM()
enum class Tipo : uint8
{
	Piedra,
	Papel,
	Tijera,
};

/** A block that can be clicked */
UCLASS(minimalapi)
class APuzzlePiedraBlock : public AActor
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

	/** StaticMesh component for the clickable block */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BlockMesh;

public:
	APuzzlePiedraBlock();

	/** Are we currently active? */
	bool bIsActive;

	/** Pointer to white material used on the focused block */
	UPROPERTY()
	class UMaterial* BaseMaterial;

	/** Pointer to blue material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* BlueMaterial;

	/** Pointer to orange material used on active blocks */
	UPROPERTY()
	class UMaterialInstance* OrangeMaterial;

	/** Grid that owns us */
	UPROPERTY()
	class APuzzlePiedraBlockGrid* OwningGrid;

	/** Handle the block being clicked */
	UFUNCTION()
	void BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked);

	/** Handle the block being touched  */
	UFUNCTION()
	void OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);

	void HandleClicked();

	void Highlight(bool bOn);

public:
	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
	/** Returns BlockMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBlockMesh() const { return BlockMesh; }


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MallaPiedra")
	UStaticMesh* MallaPiedra = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MallaPapel")
	UStaticMesh* MallaPapel = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MallaTijera")
	UStaticMesh* MallaTijera = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemigo")
	APuzzlePiedraBlock* Enemigo = nullptr;

	float Contador = 0.f;
	float Fuerza = 1000;
	bool estaRotando = false;
	UPROPERTY(VisibleAnywhere, Category = "Tipos")
	Tipo TipoActual;
	void Lanzar();
	void ActivarMetodos();
	void EstableceTipo();
	void AplicarMalla();
	void Resultado();
	FString TipoText(Tipo T);
	bool EstaIniciado = false;
	void beginRoll(bool EnemigoRotando);
	virtual void Tick(float DeltaTime) override;

	//____ additions: skin materials and audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skins")
	UMaterialInstance* PiedraMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skins")
	UMaterialInstance* PapelMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skins")
	UMaterialInstance* TijeraMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* SoundWin = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* SoundLose = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* SoundTie = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* SoundLaunch = nullptr;
	//____

};





