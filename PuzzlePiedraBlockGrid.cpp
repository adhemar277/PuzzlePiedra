// Copyright Epic Games, Inc. All Rights Reserved.

#include "PuzzlePiedraBlockGrid.h"
#include "PuzzlePiedraBlock.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "PuzzleBlockGrid"

APuzzlePiedraBlockGrid::APuzzlePiedraBlockGrid()
{
	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Create static mesh component
	ScoreText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScoreText0"));
	ScoreText->SetRelativeLocation(FVector(200.f, 0.f, 0.f));
	ScoreText->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "Score: {0}"), FText::AsNumber(0)));
	ScoreText->SetupAttachment(DummyRoot);

	// Set defaults
	Size = 3;
	BlockSpacing = 300.f;
	Score = 0; // initialize score
}


void APuzzlePiedraBlockGrid::BeginPlay()
{
	Super::BeginPlay();

	// Number of blocks
	const int32 NumBlocks = Size * Size;

	// Keep a list of spawned blocks so we can pair them as "enemigos"
	TArray<APuzzlePiedraBlock*> SpawnedBlocks;

	// Loop to spawn each block
	for (int32 BlockIndex = 0; BlockIndex < NumBlocks; BlockIndex++)
	{
		const float XOffset = (BlockIndex / Size) * BlockSpacing; // Divide by dimension
		const float YOffset = (BlockIndex % Size) * BlockSpacing; // Modulo gives remainder

		// Make position vector, offset from Grid location
		const FVector BlockLocation = FVector(XOffset, YOffset, 0.f) + GetActorLocation();

		// Spawn a block
		APuzzlePiedraBlock* NewBlock = GetWorld()->SpawnActor<APuzzlePiedraBlock>(BlockLocation, FRotator(0, 0, 0));

		// Tell the block about its owner
		if (NewBlock != nullptr)
		{
			NewBlock->OwningGrid = this;
			SpawnedBlocks.Add(NewBlock);
		}
	}

	// Pair spawned blocks as enemies in simple pairs (0<->1,2<->3, ...)
	for (int32 i = 0; i + 1 < SpawnedBlocks.Num(); i += 2)
	{
		APuzzlePiedraBlock* A = SpawnedBlocks[i];
		APuzzlePiedraBlock* B = SpawnedBlocks[i + 1];
		if (A && B)
		{
			A->Enemigo = B;
			B->Enemigo = A;
		}
	}

	// If odd number of blocks, warn that the last block has no enemy
	if (SpawnedBlocks.Num() % 2 == 1)
	{
		APuzzlePiedraBlock* Last = SpawnedBlocks.Last();
		if (Last)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] No se emparejó: sin 'Enemigo' asignado (par pool impar)."), *Last->GetName());
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Un bloque no tiene enemigo: asignalo manualmente o usa un tamaño par."));
			}
		}
	}
}


void APuzzlePiedraBlockGrid::AddScore()
{
	// Increment score
	Score++;

	// Update text
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "Score: {0}"), FText::AsNumber(Score)));
}

#undef LOCTEXT_NAMESPACE
