#include <Model.h>
#include <WorldTransform.h>
#include "MathUtilityForText.h"
#include "aabb.h"

class MapChipField;
class Enemy;

enum class LRDirection {
	kRight,
	kLeft
};

enum Corner {
	kRightBottom, // 右下
	kLeftBottom,  // 左下
	kRightTop,    // 右上
	kLeftTop,     // 左上

	kNumCorner // 要素数
};

struct CollisionMapInfo {
	// 天井衝突フラグ
	bool ceilCollision = false;
	// 着地フラグ
	bool landing = false;
	// 壁接触フラグ
	bool wallHit = false;
	// 移動量
	Vector3 moveAmount;
};

class Player {
public:
	void Initialize(Model* model, ViewProjection* viewProjection,const Vector3& position);
	void Update();
	void Draw();
	void Move();
	void MapCollision(CollisionMapInfo& info);
	void MapTopCollision(CollisionMapInfo& info);
	void MapBottomCollision(CollisionMapInfo& info);
	void MapRightCollision(CollisionMapInfo& info);
	void MapLeftCollision(CollisionMapInfo& info);
	void ResultMove(const CollisionMapInfo& info);
	void CeilingHit(const CollisionMapInfo& info);
	void WallHit(const CollisionMapInfo& info);
	void OnGround(const CollisionMapInfo& info);

	static inline const float kAcceleration = 0.05f;
	static inline const float kAttenuation = 0.5f;
	static inline const float kLimitRunSpeed = 0.8f;
	//Vector3 velocity_ = {};

	LRDirection lrDirection_ = LRDirection::kRight;
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	static inline const float kTimeTurn = 0.3f;

	bool onGround_ = true;

	static inline const float kGravityAcceleration = 0.2f;
	static inline const float kLimitFallSpeed = 0.8f;
	static inline const float kJumpAcceleration = 1.0f;

	const WorldTransform& GetWorldTransform() const { return worldTransform_; }

	const Vector3& GetVelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	void OnCollision(const Enemy* enemy);

	Vector3 CornerPosition(const Vector3& center, Corner corner);

	Vector3 GetWorldPosition();

	AABB GetAABB();

	bool isDead_ = false;

	bool IsDead() const { return isDead_; }

private:

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 1.0f;
	static inline const float kAttenuationWall = 1.0f;
	static inline const float kAttenuationLanding = 0.2f;
	static inline const float kGroundSearchHeight = 0.06f;
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* modelPlayer_ = nullptr;

	Vector3 velocity_ = {};

	MapChipField* mapChipField_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	ViewProjection* viewProjection_ = nullptr;
};