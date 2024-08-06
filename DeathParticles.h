#pragma once
#include "Model.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include "MathUtilityForText.h"
#include <array>
#include <numbers>
#include <algorithm>

class DeathParticles {

public:
	void Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position);

	void Update();

	void Draw();

	bool isFinished_ = false;

	bool IsFinished() const { return isFinished_; }

private:
	Model* model_ = nullptr;

	ViewProjection* viewProjection_ = nullptr;

	static inline const uint32_t kNumParticles = 8;

	std::array<WorldTransform, kNumParticles> worldTransforms_;

	ObjectColor objectColor_;

	Vector4 color_;

	static inline const float kDuration = 2.0f;

	static inline const float kSpeed = 0.05f;

	static inline const float kAngleUnit = (2 * std::numbers::pi_v<float> / kNumParticles);

	//bool isFinished_ = false;

	float counter_ = 0.0f;

};
