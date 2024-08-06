#define NOMINMAX
#define NOMINMIN
#include "CameraController.h"
#include "Player.h"
#include "MathUtilityForText.h"

void CameraController::Initialize() { 
	viewProjection_.Initialize();
}

void CameraController::Update() {

// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	const Vector3& targetVelocity = target_->GetVelocity();

	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	targetPos_ = targetWorldTransform.translation_ + targetOffset_ + targetVelocity * kVelocityBias;

	// 座標補間によりゆったり追従
	viewProjection_.translation_ = Lerp(viewProjection_.translation_, targetPos_, kInterpolationRate);

	// 追従対象が画面外に出ないように補正
	viewProjection_.translation_.x = std::max(viewProjection_.translation_.x, targetPos_.x + margen.left);
	viewProjection_.translation_.x = std::min(viewProjection_.translation_.x, targetPos_.x + margen.right);
	viewProjection_.translation_.y = std::max(viewProjection_.translation_.y, targetPos_.y + margen.bottom);
	viewProjection_.translation_.y = std::min(viewProjection_.translation_.y, targetPos_.y + margen.top);

	// 移動範囲制限
	viewProjection_.translation_.x = std::max(viewProjection_.translation_.x, movableArea_.left);
	viewProjection_.translation_.x = std::min(viewProjection_.translation_.x, movableArea_.right);
	viewProjection_.translation_.y = std::max(viewProjection_.translation_.y, movableArea_.bottom);
	viewProjection_.translation_.y = std::min(viewProjection_.translation_.y, movableArea_.top);

	// 行列を更新する
	viewProjection_.UpdateMatrix();
}

void CameraController::Reset() {

const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

viewProjection_.translation_ = targetWorldTransform.translation_ + targetOffset_;

}
