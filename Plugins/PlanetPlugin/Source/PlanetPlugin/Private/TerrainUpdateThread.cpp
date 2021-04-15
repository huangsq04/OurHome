// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainUpdateThread.h"

FTerrainUpdateThread::FTerrainUpdateThread()
{
	Running = true;
	Thread = FRunnableThread::Create(this, TEXT("TerrainUpdateThread"), 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

FTerrainUpdateThread::~FTerrainUpdateThread()
{
	Thread->Kill(true);
	delete Thread;
}
uint32 FTerrainUpdateThread::Run()
{
	while (Running)
	{
		RunUpdateTask();
	}
	delete this;
	return 0;
}

void FTerrainUpdateThread::RunUpdateTask()
{
	if (TerrainUpdateFun.IsBound())
	{
		TerrainUpdateFun.Execute();
		TerrainUpdateFun.Unbind();
	}
	FPlatformProcess::Sleep(0.1);

}
