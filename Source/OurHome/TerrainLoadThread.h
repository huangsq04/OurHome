// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

/**
 * 
 */
class OURHOME_API TerrainLoadThread : public FRunnable
{
public:
	TerrainLoadThread();
	~TerrainLoadThread();


	virtual bool Init() override { return true; }

	virtual uint32 Run() override;
	virtual void Stop() override {};
	virtual void Exit() override { Running = false; };

	void RunUpdateTask();

	bool Running;

	DECLARE_DELEGATE(FTerrainLoadFun);
	FTerrainLoadFun TerrainLoadFun;

	FRunnableThread* Thread;
};
