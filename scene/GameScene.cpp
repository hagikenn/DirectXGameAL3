#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
// #include "Player.h"

GameScene::GameScene() {}

GameScene::~GameScene() {
	// 各オブジェクトのメモリを解放
	delete mapChipField_;
	delete cameraController_;
	delete debugCamera_;
	delete deathParticles_;
}

void GameScene::Initialize() {
	// 各種システムのインスタンスを取得
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// モデルの読み込み
	modelBlock_ = Model::CreateFromOBJ("block", true);

	// ビュープロジェクションの初期化
	viewProjection_.Initialize();

	// マップチップの読み込み
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/map.csv");

	// ブロックの生成
	GenerateBlocks();

	// プレイヤーの初期化
	player_ = new Player();

	// プレイヤーの初期位置を設定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(3, 15);
	player_->SetMapChipField(mapChipField_);
	player_->Initialize(modelPlayerResource_, &viewProjection_, playerPosition);

	// パーティクルの初期化
	modelParticles_ = Model::CreateFromOBJ("deathParticle", true);
	deathParticles_ = new DeathParticles;
	deathParticles_->Initialize(modelParticles_, &viewProjection_, playerPosition);

	// 敵の初期化
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	for (int32_t i = 0; i < 1; ++i) {
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(50 + i * 6, 18);
		newEnemy->Initialize(modelEnemy_, &viewProjection_, enemyPosition);
		enemies_.push_back(newEnemy);
	}

	// ゲームの初期フェーズを設定
	phase_ = Phase::kPlay;

	// カメラコントローラーの初期化
	cameraController_ = new CameraController();
	cameraController_->Initialize();
	cameraController_->SetTarget(player_);

	// デバッグカメラの初期化
	debugCamera_ = new DebugCamera(1280, 720);

	// カメラの可動範囲を設定   //40のところが移動範囲を変えれるところ
	Rect cameraArea = {11.0f, 40, 6.0f, 6.0f};
	cameraController_->SetMovableArea(cameraArea);

	// カメラのリセット
	cameraController_->Reset();
}

void GameScene::GenerateBlocks() {
	// マップチップフィールドのブロック数を取得
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// ワールドトランスフォームブロックのリサイズ
	worldTransformBlocks_.resize(numBlockHorizontal);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの配置
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::CheckAllCollisions() {
#pragma region 自キャラと敵キャラの当たり判定

	// プレイヤーと敵のAABB衝突判定
	AABB aabb1, aabb2;
	aabb1 = player_->GetAABB();
	for (Enemy* enemy : enemies_) {
		aabb2 = enemy->GetAABB();
		if (AABB::IsCollision(aabb1, aabb2)) {
			// 衝突時の処理
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}
#pragma endregion
}

void GameScene::Update() {
	// デバッグカメラの更新
	debugCamera_->Update();

#ifndef DEBUG
	// デバッグカメラの有効/無効をトグル
	if (input_->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif // !DEBUG

	// ゲームのフェーズ変更を確認
	ChangePhase();

	// フェーズに応じた更新処理
	switch (phase_) {
	case Phase::kPlay:
		// プレイヤーの更新
		player_->Update();
		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		// カメラコントローラーの更新
		cameraController_->Update();
		// カメラビューの切り替え
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			viewProjection_.matView = debugCamera_->GetViewProjection().matView;
			viewProjection_.matProjection = debugCamera_->GetViewProjection().matProjection;
			viewProjection_.TransferMatrix();
		} else {
			viewProjection_.matView = cameraController_->GetViewProjection().matView;
			viewProjection_.matProjection = cameraController_->GetViewProjection().matProjection;
			viewProjection_.TransferMatrix();
		}
		// ワールドトランスフォームブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;
				worldTransformBlock->UpdateMatrix();
			}
		}
		// 衝突判定のチェック
		CheckAllCollisions();
		break;

	case Phase::kDeath:
		// 死亡フェーズの処理
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		if (deathParticles_) {
			deathParticles_->Update();
		}
		cameraController_->Update();
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			viewProjection_.matView = debugCamera_->GetViewProjection().matView;
			viewProjection_.matProjection = debugCamera_->GetViewProjection().matProjection;
			viewProjection_.TransferMatrix();
		} else {
			viewProjection_.matView = cameraController_->GetViewProjection().matView;
			viewProjection_.matProjection = cameraController_->GetViewProjection().matProjection;
			viewProjection_.TransferMatrix();
		}
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;
				worldTransformBlock->UpdateMatrix();
			}
		}
		break;
	}
}

void GameScene::Draw() {
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);
	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>
	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	// フェーズに応じた描画処理
	switch (phase_) {
	case Phase::kPlay:
		// プレイヤーの描画
		player_->Draw();
		break;
	case Phase::kDeath:
		break;
	}

	// ワールドトランスフォームブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			modelBlock_->Draw(*worldTransformBlock, viewProjection_);
		}
	}

	// 死亡パーティクルの描画
	if (deathParticles_) {
		deathParticles_->Draw();
	}

	// 敵の描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}
void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		if (player_->IsDead()) {
			// 死亡演出フェーズに切り替え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelParticles_, &viewProjection_, deathParticlesPosition);
		}
		break;
	case Phase::kDeath:
		// デス演出フェーズの処理
		if (deathParticles_ && deathParticles_->IsFinished()) {
			finished_ = true;
		}
		break;
	}
}