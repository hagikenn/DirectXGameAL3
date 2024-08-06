#pragma once

#include "Audio.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Model.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include "MapChipField.h"
#include "Player.h"
#include "Enemy.h"
#include "CameraController.h"
#include "DebugCamera.h"
#include "DeathParticles.h"


/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	bool isDebugCameraActive_ = false;

	DebugCamera* debugCamera_ = nullptr;

	MapChipField* mapChipField_;

	Model* modelBlock_ = nullptr;

	ViewProjection viewProjection_;

	Player* player_ = nullptr;

	Model* modelPlayerResource_ = nullptr;
	
	DeathParticles* deathParticles_ = nullptr;
	Model* modelParticles_ = nullptr;

	//Enemy* enemy_ = nullptr;
	std::list<Enemy*> enemies_;

	enum class Phase {

		kPlay,
		kDeath



	};
	
	Phase phase_;

	void ChangePhase();

	bool finished_ = false;

	bool IsFinished() const { return finished_; }

	Model* modelEnemy_ = nullptr;
	
	CameraController* cameraController_;

	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	void GenerateBlocks();

	void CheckAllCollisions();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;

	/// <summary>
	/// ゲームシーン用
	/// </summary>
};
