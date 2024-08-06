#include "DeathParticles.h"

void DeathParticles::Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position) {

	model_ = model;
	viewProjection_ = viewProjection;

	for (auto& worldTransform : worldTransforms_) {
	
	worldTransform.Initialize();
		worldTransform.translation_ = position;
	
	}

	objectColor_.Initialize();
	color_ = {1, 1, 1, 1};

}

void DeathParticles::Update() {

	if (isFinished_) {
		return;
	
	}

	Matrix4x4 MakeRotateZMatrix(float radian);

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		Vector3 velocity = {kSpeed, 0, 0};
		float angle = kAngleUnit * i;
		Matrix4x4 matrixRotaion = MakeRotateZMatrix(angle);
		velocity = Transform(velocity, matrixRotaion);

		worldTransforms_[i].translation_ += velocity;
	
	}

	counter_ += 1.0f / 60.0f;

	if (counter_ >= kDuration) {
	
		counter_ = kDuration;
		isFinished_ = true;
	}


	color_.w = std::clamp(1.0f - counter_ / kDuration, 0.0f, 1.0f);
	objectColor_.SetColor(color_);
	objectColor_.TransferMatrix();


	for (auto& worldTransform : worldTransforms_) {
		worldTransform.UpdateMatrix();
	}
	

}

void DeathParticles::Draw() {
	if (isFinished_) {
		return;
	}

	for (auto& worldTransform : worldTransforms_) {
		model_->Draw(worldTransform, *viewProjection_, &objectColor_);
	}


}
