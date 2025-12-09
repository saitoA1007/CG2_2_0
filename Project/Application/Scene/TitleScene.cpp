#include"TitleScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"GameParamEditor.h"
#include"SpriteRenderer.h"
#include"FPSCounter.h"

#include "TDGameScene.h"
#include"Application/Stage/StageManager.h"
#include"Application/Stage/StageWallPlane.h"
#include"Application/Player/Player.h"
#include"Animator.h"
#include"Application/Graphics/PlaneProjectionShadow.h"
#include"Application/Graphics/Terrain.h"
#include"Extension/CustomMaterial/IceMaterial.h"
#include"Application/Light/SceneLightingController.h"
#include"Application/Graphics/BgRock.h"
#include"Extension/CustomRenderer.h"

#include "Application/Player/Effect/PlayerChargeEffect.h"
#include "Application/Player/Effect/PlayerRushEffect.h"
#include "Application/Player/Effect/PlayerAttackDownEffect.h"

#include "Application/Camera/CameraController.h"
#include "EngineSource/Math/EasingManager.h"
#include "EngineSource/Math/MyMath.h"
#include <numbers>

using namespace GameEngine;

TitleScene::~TitleScene() {
}

void TitleScene::Initialize(SceneContext* context) {
    // ゲームシーンに必要な低レイヤー機能
    #pragma region SceneSystem
    // エンジン機能を取得
    context_ = context;
    // 登録するパラメータを設定
    GameParamEditor::GetInstance()->SetActiveScene("TitleScene");

    // デバック用描画の初期化
    debugRenderer_ = DebugRenderer::Create();

    // 当たり判定の管理システムを初期化
    collisionManager_ = std::make_unique<CollisionManager>();
    collisionManager_->ClearList();
    #pragma endregion

    // 入力コマンド登録
    InputRegisterCommand();

    // メインカメラの初期化
    mainCamera_ = std::make_unique<Camera>();
    mainCamera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720, context_->graphicsDevice->GetDevice());

    // タイトル画像
    titleSprite_ = Sprite::Create({ 640.0f,250.0f }, { 1024.0f,256.0f }, { 0.5f,0.5f });
    titleGH_ = context_->textureManager->GetHandleByName("titleText.png");

    // スペースボタン
    spaceSprite_ = Sprite::Create({ 640.0f,500.0f }, { 256.0f,64.0f }, { 0.5f,0.5f });
    spaceGH_ = context_->textureManager->GetHandleByName("spaceText.png");

    // ライトの生成
    sceneLightingController_ = std::make_unique<SceneLightingController>();
    sceneLightingController_->Initialize(context_->graphicsDevice->GetDevice());

    icePlaneModel_ = context_->modelManager->GetNameByModel("PlaneXZ");
    terrain_ = std::make_unique<Terrain>();
    terrain_->Initialize(context_->textureManager->GetHandleByName("gras.png"), context_->textureManager->GetHandleByName("ice.png"), context_->textureManager->GetHandleByName("iceNormal.png"));

    stageManager_ = std::make_unique<StageManager>();
    stageManager_->Initialize();

    stageWallPlaneModel_ = context_->modelManager->GetNameByModel("PlaneXZ");
    stageWallPlaneMaterial_ = std::make_unique<IceMaterial>();
    stageWallPlaneMaterial_->Initialize();
    {
        IceMaterial* terrainMat = terrain_->GetMaterial();
        if (terrainMat) {
            stageWallPlaneMaterial_->materialData_->baseTextureHandle = terrainMat->materialData_->baseTextureHandle;
            stageWallPlaneMaterial_->materialData_->textureHandle = terrainMat->materialData_->textureHandle;
            stageWallPlaneMaterial_->materialData_->normalTextureHandle = terrainMat->materialData_->normalTextureHandle;
            stageWallPlaneMaterial_->materialData_->baseColor = terrainMat->materialData_->baseColor;
            stageWallPlaneMaterial_->materialData_->color = terrainMat->materialData_->color;
            stageWallPlaneMaterial_->materialData_->specularColor = terrainMat->materialData_->specularColor;
            stageWallPlaneMaterial_->materialData_->shininess = terrainMat->materialData_->shininess;
        }
    }
#ifdef _DEBUG
    StageWallPlane::RegisterDebugParam(stageWallPlaneMaterial_.get());
#else
    StageWallPlane::ApplyDebugParam(stageWallPlaneMaterial_.get());
#endif

    {
        auto& walls = stageManager_->GetWalls();
        size_t count = std::min(stageWallPlanes_.size(), walls.size());
        for (size_t i = 0; i < count; ++i) {
            const Transform& wallTransform = walls[i]->GetWorldTransform().transform_;
            Transform t = wallTransform;
            t.rotate.y += static_cast<float>(3.14159265358979323846);
            t.rotate.x -= static_cast<float>(3.14159265358979323846) / 2.0f;
            t.scale.z = 32.0f;
            t.translate.y = 16.0f;
            stageWallPlanes_[i].Initialilze(t);
        }
        for (size_t i = count; i < stageWallPlanes_.size(); ++i) {
            Transform t; t.scale = { 0.0f,0.0f,0.0f };
            stageWallPlanes_[i].Initialilze(t);
        }
    }

    playerModel_ = context_->modelManager->GetNameByModel("Player");
    playerModel_->SetDefaultIsEnableLight(true);
    playerAnimator_ = std::make_unique<Animator>();
    playerAnimationData_[static_cast<size_t>(PlayerAnimationType::Walk)] = context_->animationManager->GetNameByAnimations("PlayerWalk");
    playerAnimationData_[static_cast<size_t>(PlayerAnimationType::Rush)] = context_->animationManager->GetNameByAnimations("PlayerRush");
    playerAnimationData_[static_cast<size_t>(PlayerAnimationType::DownAttack)] = context_->animationManager->GetNameByAnimations("PlayerDownAttack");
    playerAnimationData_[static_cast<size_t>(PlayerAnimationType::AirMove)] = context_->animationManager->GetNameByAnimations("PlayerAirMove");

    playerAnimator_->Initialize(playerModel_, &playerAnimationData_[static_cast<size_t>(PlayerAnimationType::Walk)]["歩き"]);
    player_ = std::make_unique<Player>();
    player_->Initialize(playerAnimator_.get(), playerAnimationData_);
    player_->GetWorldTransform().transform_.rotate.y = 0.0f;
    player_->GetWorldTransform().UpdateTransformMatrix();

    playerShadow_ = std::make_unique<PlaneProjectionShadow>();
    playerShadow_->Initialize(&player_->GetWorldTransform());

    wallModel_ = context_->modelManager->GetNameByModel("Wall");

    playerChargeEffectModel_ = context_->modelManager->GetNameByModel("RushWave");
    playerRushEffectModel_ = playerChargeEffectModel_;
    playerAttackDownEffectModel_ = playerChargeEffectModel_;

    playerChargeEffect_ = std::make_unique<PlayerChargeEffect>();
    playerChargeEffect_->Initialize();
    playerChargeEffect_->SetParent(&player_->GetWorldTransform());

    playerRushEffect_ = std::make_unique<PlayerRushEffect>();
    playerRushEffect_->Initialize();
    playerRushEffect_->SetParent(&player_->GetWorldTransform());

    playerAttackDownEffect_ = std::make_unique<PlayerAttackDownEffect>();
    playerAttackDownEffect_->Initialize();
    playerAttackDownEffect_->SetParent(&player_->GetWorldTransform());

    player_->SetOnWallHit([this]() {
        if (cameraController_) {
            cameraController_->StartCameraShake(3.0f, 0.5f, 1000.0f,
                [](const Vector3 &a, const Vector3 &b, float t) { return Lerp(a, b, t); },
                CameraController::ShakeOrigin::TargetPosition,
                true, false, true, false);
        }
    });

    cameraController_ = std::make_unique<CameraController>();
    cameraController_->Initialize();

    transitionStartTarget_ = Vector3{ 0.0f,32.0f,0.0f };
    transitionEndTarget_ = player_ ? player_->GetWorldTransform().GetWorldPosition() : transitionStartTarget_;

    // cameraController のターゲットを初期注視点に設定して内部状態を同期
    cameraController_->SetTarget(transitionStartTarget_);
    cameraController_->SetCameraCoordinateType(CameraController::CameraCoodinateType::Spherical);
    cameraController_->SetDesiredFov(0.7f);
    cameraController_->SetDesiredAsCurrent();
    cameraController_->Update(context_->inputCommand, context_->input);
    cameraController_->SetCurrentAsDesired();
    cameraController_->Update(context_->inputCommand, context_->input);
    mainCamera_->SetCamera(cameraController_->GetCamera());
    mainCamera_->Update();

    isTitleLocked_ = true;
    isTransitioning_ = false;
    transitionTimer_ = 0.0f;
}

void TitleScene::Update() {
    // ロック中で遷移中でなければ Start のみ検出
    if (isTitleLocked_ && !isTransitioning_) {
        if (context_->inputCommand->IsCommandActive("Start")) {
            // CameraController のアニメーションを使って遷移を開始
            isTransitioning_ = true;
            transitionTimer_ = 0.0f;
        }
    } else {
        // ロック解除後は Start でシーン遷移
        if (!isTitleLocked_ && context_->inputCommand->IsCommandActive("Start")) {
            isFinished_ = true;
            TDGameScene::SetIsFirstGameStart(true);
        }
    }

    if (debugRenderer_) debugRenderer_->Clear();
    if (collisionManager_) collisionManager_->ClearList();

    if (cameraController_) {
        if (!isTitleLocked_) {
            float desiredFov = 0.7f;
            if (player_ && player_->IsRushing()) {
                desiredFov = 1.0f;
            } else if (player_ && (player_->IsCharging() || player_->IsPreRushing())) {
                int rushLevel = player_->GetRushChargeLevel();
                desiredFov = 0.4f + static_cast<float>(3 - rushLevel) * 0.1f;
            }
            cameraController_->SetDesiredFov(desiredFov);
            cameraController_->SetTarget(player_->GetWorldTransform().GetWorldPosition());
        }
        cameraController_->Update(context_->inputCommand, context_->input);
        mainCamera_->SetCamera(cameraController_->GetCamera());
    }

    // 遷移中は注視点をイージングで変化させる
    if (isTitleLocked_ && isTransitioning_) {
        transitionTimer_ += FpsCounter::deltaTime;
        float t = std::clamp(transitionTimer_ / kTransitionDuration_, 0.0f, 1.0f);
        float eased = EaseInOutCubic(0.0f, 1.0f, t);
        Vector3 currentTarget = Lerp(transitionStartTarget_, transitionEndTarget_, eased);
        // カメラコントローラーに注視点を設定
        cameraController_->SetTarget(currentTarget);

        // スプライトのフェード
        float alpha = 1.0f - eased;
        titleSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, alpha));
        spaceSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, alpha));

        if (transitionTimer_ >= kTransitionDuration_) {
            isTransitioning_ = false;
            isTitleLocked_ = false;
        }
    }

    // ライトの更新
    if (sceneLightingController_) sceneLightingController_->Update();

    if (terrain_) terrain_->Update();

    if (stageManager_) stageManager_->Update();

    // タイトルロック中はプレイヤーの操作/物理更新を行わない
    if (!isTitleLocked_) {
        if (player_) player_->Update(context_->inputCommand, *mainCamera_);
        if (playerShadow_) playerShadow_->Update();

        if (playerChargeEffect_) playerChargeEffect_->Update();
        if (playerRushEffect_) playerRushEffect_->Update();
        if (playerAttackDownEffect_) playerAttackDownEffect_->Update();
    } else {
        // ロック中でも影やエフェクトの位置は更新
        if (playerShadow_) playerShadow_->Update();
    }

    for (auto &plane : stageWallPlanes_) {
        plane.Update();
    }

    UpdateCollision();

    if (isFirstUpdate_) {
        cameraController_->SetDesiredAsCurrent();
        mainCamera_->SetCamera(cameraController_->GetCamera());
        isFirstUpdate_ = false;
    }
}

void TitleScene::Draw(const bool& isDebugView) {

    // 描画に使用するカメラを設定
    if (isDebugView) {
        // 描画に使用するカメラを設定
        ModelRenderer::SetCamera(context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
    } else {
        // 描画に使用するカメラを設定
        ModelRenderer::SetCamera(mainCamera_->GetVPMatrix(), mainCamera_->GetCameraResource());
    }

    if (isDebugView) {
        CustomRenderer::SetCamera(context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
    } else {
        CustomRenderer::SetCamera(mainCamera_->GetVPMatrix(), mainCamera_->GetCameraResource());
    }

    //===========================================================
    // 3D描画
    //===========================================================

    // アニメーションの描画前処理
    ModelRenderer::PreDraw(RenderMode3D::AnimationModel);

    // プレイヤーのアニメーションを描画
    ModelRenderer::DrawAnimationWithLight(playerModel_, player_->GetWorldTransform(), sceneLightingController_->GetResource());
    // プレイヤーの影を描画する
    ModelRenderer::DrawAnimation(playerModel_, playerShadow_->GetWorldTransform(), &playerShadow_->GetMaterial());

    // 氷のテスト描画
    CustomRenderer::PreDraw(CustomRenderMode::Ice);
    CustomRenderer::DrawIce(icePlaneModel_, terrain_->GetWorldTransform(), sceneLightingController_->GetResource(), terrain_->GetMaterial());
    for (auto &plane : stageWallPlanes_) {
        if (plane.GetIsAlive()) {
            CustomRenderer::DrawIce(icePlaneModel_, plane.GetWorldTransform(), sceneLightingController_->GetResource(), stageWallPlaneMaterial_.get());
        }
    }

    // 通常モデルの描画前処理
    ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

    // ステージを描画する
    stageManager_->Draw(wallModel_);

    // 3Dモデルの両面描画前処理
    ModelRenderer::PreDraw(RenderMode3D::DefaultModelBoth);

    // プレイヤーのエフェクト描画
    if (!isTitleLocked_) {
        if (player_->IsCharging()) {
            for (auto &chargeEffect : playerChargeEffect_->GetWorldTransforms()) {
                ModelRenderer::Draw(playerChargeEffectModel_, chargeEffect);
            }
        }
        if (player_->IsRushing()) {
            for (auto &rushEffect : playerRushEffect_->GetWorldTransforms()) {
                ModelRenderer::Draw(playerRushEffectModel_, rushEffect);
            }
        }
        if (player_->IsAttackDown()) {
            for (auto &attackDownEffect : playerAttackDownEffect_->GetWorldTransforms()) {
                ModelRenderer::Draw(playerAttackDownEffectModel_, attackDownEffect);
            }
        }
    }

    // インスタンシング描画前処理
    ModelRenderer::PreDraw(RenderMode3D::Instancing);

    // 壁破壊のパーティクルを描画
    for (auto &particle : stageManager_->GetBreakWallParticles()) {
        ModelRenderer::DrawInstancing(wallModel_, particle->GetCurrentNumInstance(), *particle->GetWorldTransforms());
    }
}

void TitleScene::DrawUI() {
    //======================================================
    // 2D描画
    //======================================================

    SpriteRenderer::PreDraw(RenderMode2D::Normal);
    SpriteRenderer::Draw(titleSprite_.get(), titleGH_);
    SpriteRenderer::Draw(spaceSprite_.get(), spaceGH_);
}

void TitleScene::InputRegisterCommand() {
    // 移動の入力コマンドを登録する
    context_->inputCommand->RegisterCommand("MoveUp", { {InputState::KeyPush, DIK_W },{InputState::PadLeftStick,0,{0.0f,1.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_UP } });
    context_->inputCommand->RegisterCommand("MoveDown", { {InputState::KeyPush, DIK_S },{InputState::PadLeftStick,0,{0.0f,-1.0f},0.2f}, {InputState::PadPush, XINPUT_GAMEPAD_DPAD_DOWN} });
    context_->inputCommand->RegisterCommand("MoveLeft", { {InputState::KeyPush, DIK_A },{InputState::PadLeftStick,0,{-1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_LEFT } });
    context_->inputCommand->RegisterCommand("MoveRight", { {InputState::KeyPush, DIK_D },{InputState::PadLeftStick,0,{1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_RIGHT } });
    // ジャンプコマンドを登録する
    context_->inputCommand->RegisterCommand("Jump", { {InputState::KeyTrigger, DIK_SPACE},{InputState::PadTrigger, XINPUT_GAMEPAD_A} });

    // AttackDownコマンド
    context_->inputCommand->RegisterCommand("AttackDown", { {InputState::MouseTrigger, 0}, {InputState::PadTrigger, XINPUT_GAMEPAD_X} });
    // RushChargeコマンド
    context_->inputCommand->RegisterCommand("RushCharge", { {InputState::MousePush, 0}, {InputState::PadPush, XINPUT_GAMEPAD_X} });
    // RushStartコマンド
    context_->inputCommand->RegisterCommand("RushStart", { {InputState::MouseRelease, 0}, {InputState::PadRelease, XINPUT_GAMEPAD_X} });

    // トリガーのテスト
    context_->inputCommand->RegisterCommand("TestTrigger", { {InputState::PadLeftTriggerTrigger,0,{0.0f,0.0f},0.2f} });

    // ロックオンコマンド
    context_->inputCommand->RegisterCommand("LockOnBoss", { {InputState::KeyTrigger, DIK_TAB }, {InputState::PadRightTriggerTrigger,0,{0.0f,0.0f},0.2f} });

    // カメラ操作のコマンドを登録する
    context_->inputCommand->RegisterCommand("CameraMoveLeft", { { InputState::KeyPush, DIK_LEFT },{InputState::PadRightStick,0,{-1.0f,0.0f},0.2f} });
    context_->inputCommand->RegisterCommand("CameraMoveRight", { { InputState::KeyPush, DIK_RIGHT },{InputState::PadRightStick,0,{1.0f,0.0f},0.2f} });

    // Startゲーム開始（EnterキーまたはAボタン）
    context_->inputCommand->RegisterCommand("Start", { {InputState::KeyTrigger, DIK_RETURN}, {InputState::PadTrigger, XINPUT_GAMEPAD_A},{InputState::KeyTrigger, DIK_SPACE} });
    // メニュー移動
    context_->inputCommand->RegisterCommand("Up", { {InputState::KeyTrigger, DIK_UP}, {InputState::PadTrigger, XINPUT_GAMEPAD_DPAD_UP} });
    context_->inputCommand->RegisterCommand("Down", { {InputState::KeyTrigger, DIK_DOWN}, {InputState::PadTrigger, XINPUT_GAMEPAD_DPAD_DOWN} });
    // バック/終了
    context_->inputCommand->RegisterCommand("Back", { {InputState::KeyTrigger, DIK_ESCAPE}, {InputState::PadTrigger, XINPUT_GAMEPAD_B} });
}

void TitleScene::UpdateCollision() {
    if (!collisionManager_ || !player_) return;

    collisionManager_->AddCollider(player_->GetCollider());
#ifdef _DEBUG
    if (debugRenderer_) debugRenderer_->AddSphere(player_->GetSphereData());
#endif

    // 床の当たり判定
    if (terrain_) collisionManager_->AddCollider(terrain_->GetCollider());

    // ステージの壁の当たり判定
    const std::vector<Wall*> aliveWalls = stageManager_->GetAliveWalls();
    for (auto& wall : aliveWalls) {
        collisionManager_->AddCollider(wall->GetCollider());
#ifdef _DEBUG
        if (debugRenderer_) debugRenderer_->AddBox(wall->GetOBBData());
#endif
    }

    // 衝突判定を実行
    collisionManager_->CheckAllCollisions();
}
