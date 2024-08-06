#include "affine.h"
#include <math.h>

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 ret = {};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				ret.mat[i][j] += m1.mat[i][k] * m2.mat[k][j];
			}
		}
	}
	return ret;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate) {

   Matrix4x4 ScallMat, RotateMat, RotateMatX, RotateMatY, RotateMatZ, TranslateMat, returnMat;

	// スケール行列作成
	ScallMat = {scale.x, 0, 0, 0, 0, scale.y, 0, 0, 0, 0, scale.z, 0, 0, 0, 0, 1};

	// XYZ回転行列作成
	RotateMatX = {1, 0, 0, 0, 0, cosf(rot.x), sinf(rot.x), 0, 0, -sinf(rot.x), cosf(rot.x), 0, 0, 0, 0, 1};

	RotateMatY = {cosf(rot.y), 0, -sinf(rot.y), 0, 0, 1, 0, 0, sinf(rot.y), 0, cosf(rot.y), 0, 0, 0, 0, 1};

	RotateMatZ = {cosf(rot.z), sinf(rot.z), 0, 0, -sinf(rot.z), cosf(rot.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	// XYZ回転行列の合成(Z*X*Y)
	RotateMat = Multiply(RotateMatZ, RotateMatX);
	// ↑の結果＊Y軸回転
	RotateMat = Multiply(RotateMat, RotateMatY);

	// 平行移動行列作成
	TranslateMat = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, translate.x, translate.y, translate.z, 1};

	// スケール＊回転＊平行移動をワールド変換行列に
	returnMat = Multiply(ScallMat, RotateMat);
	returnMat = Multiply(returnMat, TranslateMat);

	return returnMat;
}