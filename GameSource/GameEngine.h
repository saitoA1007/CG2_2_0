#pragma once
#include"EngineSource/Core/WindowsApp.h"
#include"EngineSource/Core/DirectXCommon.h"
#include"EngineSource/Core/TextureManager.h"
#include"EngineSource/Core/PSO/LinePSO.h"
#include"EngineSource/Core/PSO/TrianglePSO.h"
#include"EngineSource/Core/PSO/ParticlePSO.h"
#include"EngineSource/Core/PSO/PostProcessPSO.h"

#include"EngineSource/Common/LogManager.h"
#include"EngineSource/Common/ResourceLeakChecker.h"
#include"EngineSource/Common/CrashHandle.h"

#include"EngineSource/3D/Camera/Camera.h"
#include"EngineSource/3D/Camera/DebugCamera.h"
#include"EngineSource/3D/Light/LightManager.h"
#include"EngineSource/3D/Model.h"
#include"EngineSource/3D/WorldTransform.h"
#include"EngineSource/3D/WorldTransforms.h"
#include"EngineSource/3D/AxisIndicator.h"
#include"EngineSource/3D/PrimitiveRenderer.h"
#include"EngineSource/3D/LineMesh.h"

#include"EngineSource/2D/ImGuiManager.h"
#include"EngineSource/2D/Sprite.h"

#include"EngineSource/Math/TransformationMatrix.h"

#include"EngineSource/Input/InPut.h"

#include"EngineSource/Audio/AudioManager.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")