#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
//#include "Player.h"


GameScene::GameScene() {}



GameScene::~GameScene() {

	delete mapChipField_;
	delete cameraController_;
	delete debugCamera_;
	delete deathParticles_;

}

void GameScene::Initialize() {

	
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	
	

	modelBlock_ = Model::CreateFromOBJ("block", true);

	viewProjection_.Initialize();

	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/map.csv");

	GenerateBlocks();



	player_ = new Player();


	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(3, 15);
	

	player_->SetMapChipField(mapChipField_);


	player_->Initialize(modelPlayerResource_, &viewProjection_, playerPosition);


	
	//enemy_ = new Enemy();
	modelParticles_ = Model::CreateFromOBJ("deathParticle", true);

	deathParticles_ = new DeathParticles;

	deathParticles_->Initialize(modelParticles_, &viewProjection_, playerPosition);

	
	
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	
	for (int32_t i = 0; i < 1; ++i) {
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(10 + i * 3, 18);
		newEnemy->Initialize(modelEnemy_, &viewProjection_, enemyPosition);
		enemies_.push_back(newEnemy);
	}

	phase_ = Phase::kPlay;


	cameraController_ = new CameraController();
	cameraController_->Initialize();
	cameraController_->SetTarget(player_);


	debugCamera_ = new DebugCamera(1280, 720);

	// カメラ
	cameraController_ = new CameraController();
	cameraController_->Initialize();
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	Rect cameraArea = {11.0f, 20, 6.0f, 6.0f};
	cameraController_->SetMovableArea(cameraArea);


	cameraController_->Reset();

}

void GameScene::GenerateBlocks() {

	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockHorizontal);

	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

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
	
	AABB aabb1, aabb2;
	
	aabb1 = player_->GetAABB();
	
	for (Enemy* enemy : enemies_) {
		
		aabb2 = enemy->GetAABB();
		if (AABB::IsCollision(aabb1, aabb2)) {
			
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}
#pragma endregion
}


void GameScene::Update() {
	debugCamera_->Update();
	#ifndef DEBUG
	
	
	

	if (input_->TriggerKey(DIK_0)) {
		if (isDebugCameraActive_ == true) {
		
		isDebugCameraActive_ = false;

		} else {
			isDebugCameraActive_ = true;
		}
	
	}

	

#endif // !DEBUG


		ChangePhase();
	switch (phase_) {
	

	case Phase::kPlay:
		player_->Update();//プレイヤー
		for (Enemy* enemy : enemies_) {
			enemy->Update();// 
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
		CheckAllCollisions();
		break;

	case Phase::kDeath:

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

	switch (phase_) {
	case Phase::kPlay:

		player_->Draw();

		break;
	case Phase::kDeath:
		break;
	}


	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;

			modelBlock_->Draw(*worldTransformBlock, viewProjection_);
		}
	}
	
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