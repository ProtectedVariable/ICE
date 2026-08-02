#pragma once

// ICE engine umbrella header: the single discoverable front door. A hello-world application can
//
//   #include <ICE.h>
//
// and reach the engine, scenes, the behaviour context, components, input, the UI, and the render-
// feature/handle API without hunting through module headers. It is purely additive -- every module
// header below still exists and can be included directly for a leaner compile.

// --- Engine ------------------------------------------------------------------------------------
#include <ICEEngine.h>
#include <Project.h>
#include <AssetBank.h>
#include <Logger.h>
#include <ICEMath.h>

// --- Scene & entities --------------------------------------------------------------------------
#include <Scene.h>
#include <SceneCamera.h>
#include <EntityHandle.h>
#include <Camera.h>
#include <PerspectiveCamera.h>
#include <OrthographicCamera.h>

// --- Components --------------------------------------------------------------------------------
#include <TransformComponent.h>
#include <RenderComponent.h>
#include <LightComponent.h>
#include <CameraComponent.h>
#include <AnimationComponent.h>
#include <SkyboxComponent.h>

// --- Behaviour scripting (T3) ------------------------------------------------------------------
#include <NativeScript.h>
#include <NativeScriptComponent.h>

// --- Input (T2) --------------------------------------------------------------------------------
#include <InputManager.h>

// --- Render features & typed resource handles (T4/T5) ------------------------------------------
#include <RenderFeature.h>
#include <RenderGraph.h>

// --- UI (T9) -----------------------------------------------------------------------------------
#include <UI.h>
