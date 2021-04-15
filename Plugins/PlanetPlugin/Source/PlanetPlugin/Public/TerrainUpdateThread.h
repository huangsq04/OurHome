// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/RunnableThread.h"

/**
 * 
 */
class PLANETPLUGIN_API FTerrainUpdateThread  : public FRunnable
{
public:
	FTerrainUpdateThread();
	~FTerrainUpdateThread();
	virtual bool Init() override { return true; }

	virtual uint32 Run() override;
	virtual void Stop() override {};
	virtual void Exit() override { Running = false; };

	void RunUpdateTask();

	bool Running;

	DECLARE_DELEGATE(FTerrainUpdateFun);
	FTerrainUpdateFun TerrainUpdateFun;

	FRunnableThread* Thread;
};
