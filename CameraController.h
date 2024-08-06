#pragma once
#include "ViewProjection.h"
#include "MathUtilityForText.h"

class Player;


struct Rect {
	float left = 0.0f;   // 左端
	float right = 1.0f;  // 右端
	float bottom = 0.0f; // 下端
	float top = 1.0f;    // 上端
};

class CameraController {

public:

	
	void Initialize();

	void Update();

	void SetTarget(Player* target) { target_ = target; }

	void Reset();

	void SetMovableArea(Rect area) { movableArea_ = area; } // セッター完成
	// 追従対象とカメラの座標の差（オフセット）
	Vector3 targetOffset_ = {0, 0, -15.0f};
	const ViewProjection& GetViewProjection() const { return viewProjection_; }

	Rect movableArea_ = {0, 100, 0, 100};

	//Vector3 targetOffset_ = {0, 0, -15.0f};

	Vector3 targetPos_;

private:

	ViewProjection viewProjection_;

	Player* target_ = nullptr;

	static inline const float kInterpolationRate = 0.8f;
	const float kVelocityBias = 1.0f;
	static inline const Rect margen = {-5.0f, 5.0f, -5.0f, 5.0f};

};
