#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "Aabb.h"

class Player;
class Enemy {

public:
	Enemy();
	~Enemy();

	void Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position);
	void Update();
	void Draw();
	void OnCollision(const Player* player);
	Vector3 GetWorldPosition();
	AABB GetAABB();

	private:
	
	static inline const float kWidth = 1.0f;
	    static inline const float kHeight = 1.0f;

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.03f;
	// 最初の角度
	static inline const float kWalkMotionAngleStart = 5.0f;
	// 最後の角度
	static inline const float kWalkMotionAngleEnd = -5.0f;
	// アニメーションの周期となる時間
	static inline const float kWalkMotionTime = 1.0f;
	// 経過時間
	float walkTimer_ = 0.0f;

	// 速度
	Vector3 velocity_ = {};
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// ビュープロジェクション
	ViewProjection* viewProjection_ = nullptr;

};
