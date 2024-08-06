#pragma once
#define NOMINMAX
#include <numbers>
#include "Player.h"
#include "cassert"
#include "MapChipField.h"
#include <Input.h>
#include <algorithm>
#include "ViewProjection.h"
#include "WorldTransform.h"



void Player::Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position) {
	// NULLポインタチェック
	/*assert(model);*/
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	modelPlayer_ = model;

	modelPlayer_ = Model::CreateFromOBJ("player",true);
	
	
	

	/*textureHandle_ = textureHandle;*/
	// 引数の内容をメンバ変数に記録
	viewProjection_ = viewProjection;
}

void Player::Update() {

	Move();

	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.moveAmount = velocity_;

	MapCollision(collisionMapInfo);

	ResultMove(collisionMapInfo);

	//CeilingHit(collisionMapInfo);
	
	OnGround(collisionMapInfo);

	//WallHit(collisionMapInfo);
	
	//velocity_ = collisionMapInfo.moveAmount;

	//worldTransform_.translation_.x += velocity_.x;
	//worldTransform_.translation_.y += velocity_.y;
	//worldTransform_.translation_.z += velocity_.z;

	// 行列計算
	worldTransform_.UpdateMatrix();
	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}


void Player::Draw() {
	// 3Dモデルを描画
	modelPlayer_->Draw(worldTransform_, *viewProjection_, textureHandle_);
}

void Player::Move() {

bool landing = false;

	if (velocity_.y < 0) {

		if (worldTransform_.translation_.y <= 1.0f) {
			landing = true;
		}
	}

	if (onGround_) {

		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {

			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {

				if (lrDirection_ != LRDirection::kRight) {

					lrDirection_ = LRDirection::kRight;

					// worldTransform_.rotation_.y = turnFirstRotationY_;
					turnFirstRotationY_ = worldTransform_.rotation_.y;

					turnTimer_ = kTimeTurn;
				}

				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x += kAcceleration;

			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}

				if (velocity_.x > 0.0f) {

					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration;
			}
			velocity_.x += acceleration.x;
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {
			velocity_.x *= (1.0f - kAttenuation);
			// velocity_.z *= (1.0f - kAttenuation);
		}

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			velocity_ += Vector3(0, kJumpAcceleration, 0);
		}

	} else {
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}

	if (turnTimer_ > 0.0f) {

		turnTimer_ -= 1.0f / 60.0f;

		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};

		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}

	if (velocity_.y > 0.0f) {
		onGround_ = false;

	} else {

		if (landing) {

			worldTransform_.translation_.y = 1.0f;
			velocity_.x *= (1.0f - kAttenuation);
			velocity_.y = 0.0f;
			onGround_ = true;
		}
	}

}
Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {kWidth / 2.0f,  -kHeight / 2.0f, 0}, //  kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kLeftBottom
	    {kWidth / 2.0f,  kHeight / 2.0f,  0}, //  kRightTop
	    {-kWidth / 2.0f, kHeight / 2.0f,  0}  //  kLeftTop
	};
	Vector3 result = center + offsetTable[static_cast<uint32_t>(corner)];
	
	return result;
}


// マップ衝突判定上
void Player::MapTopCollision(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.moveAmount.y <= 0) {
		return;
	}
	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
	}
	MapChipType mapChipType;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		//indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.moveAmount.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
		// 天井に当たったことを記録する
		info.ceilCollision = true;
	}
}
// マップ衝突判定下
void Player::MapBottomCollision(CollisionMapInfo& info) {
	// 下降あり？
	if (info.moveAmount.y >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		Vector3 center = worldTransform_.translation_ + info.moveAmount;
		positionsNew[i] = CornerPosition(center, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	bool hit = false;
	// 真下の当たり判定を行う
	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, -kHeight / 2.0f, 0));

		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.moveAmount + Vector3(0, -kHeight / 2.0f, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.y = std::min(0.0f, (worldTransform_.translation_.y - kHeight / 2.0f) - rect.top);
			info.landing = true;
		}
	}
}
// マップ衝突判定右
void Player::MapRightCollision(CollisionMapInfo& info) {
	// 右移動？
	if (info.moveAmount.x <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	bool hit = false;
	// 右下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(+kWidth / 2.0f, 0, 0));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.moveAmount.x = std::min(0.0f, worldTransform_.translation_.x - rect.left + (kWidth / 2.0f + kBlank));
		// 壁に当たったことを記録する
		info.wallHit = true;
	}
}

// マップ衝突判定左
void Player::MapLeftCollision(CollisionMapInfo& info) {
	// 左移動？
	if (info.moveAmount.x >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	bool hit = false;
	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(-kWidth / 2.0f, 0, 0));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.moveAmount.x = std::max(0.0f, worldTransform_.translation_.x - rect.right - (kWidth / 2.0f + kBlank));
		// 壁に当たったことを記録する
		info.wallHit = true;
	}
}



void Player::MapCollision(CollisionMapInfo& info) {
	MapTopCollision(info);
	MapBottomCollision(info);
	MapRightCollision(info);
	MapLeftCollision(info);
}
// 判定結果を反映して移動させる
void Player::ResultMove(const CollisionMapInfo& info) {
	// 移動
	worldTransform_.translation_ += info.moveAmount;
}

// 天井に接触している場合の処理
void Player::CeilingHit(const CollisionMapInfo& info) {
	// 天井に当たった？
	if (info.ceilCollision) {
		velocity_.y = 0;
	}
}
// 壁に接触してる場合の処理
void Player::WallHit(const CollisionMapInfo& info) {
	// 壁接触による減速
	if (info.wallHit) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::OnGround(const CollisionMapInfo& info) {

	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {

			std::array<Vector3, kNumCorner> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
			}

			bool hit = false;

			MapChipType mapChipType;
			// 左下点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 落下開始
			if (!hit) {
				onGround_ = false;
			}
		}
	} else {
		// 着地
		if (info.landing) {
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
			onGround_ = true;
		}
	}
}

Vector3 Player::GetWorldPosition() { 

	Vector3 worldPos;
	worldPos.x = worldTransform_.translation_.x;
	worldPos.y = worldTransform_.translation_.y;
	worldPos.z = worldTransform_.translation_.z;

	return worldPos;

}

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	// ジャンプ歌詞(仮処理)
//	velocity_.y += 1.0f;
	isDead_ = true;
}

AABB Player::GetAABB() {

	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;

}
