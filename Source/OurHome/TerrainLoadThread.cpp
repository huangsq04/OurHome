// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainLoadThread.h"

#pragma optimize("", off)
TerrainLoadThread::TerrainLoadThread()
{
	Running = true;
	Thread = FRunnableThread::Create(this, TEXT("TerrainLoadThread"), 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

TerrainLoadThread::~TerrainLoadThread()
{
	Thread->Kill(true);
	delete Thread;
}
uint32 TerrainLoadThread::Run()
{
	while (Running)
	{
		RunUpdateTask();
	}
	delete this;
	return 0;
}

void TerrainLoadThread::RunUpdateTask()
{
	if (TerrainLoadFun.IsBound())
	{
		TerrainLoadFun.Execute();
		TerrainLoadFun.Unbind();
	}
	FPlatformProcess::Sleep(0.1);

}
#pragma optimize("", on)
