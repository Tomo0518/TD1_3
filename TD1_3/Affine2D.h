#pragma once
#include "Matrix3x3.h"
#include "Vector2.h"

class AffineMatrix2D : public Matrix3x3 {

public:

	// 拡大縮小行列
	static AffineMatrix2D MakeScaleMatrix(const Vector2 scale);
	static AffineMatrix2D MakeRotationMatrix(float theta);
	static AffineMatrix2D MakeTranslateMatrix(const Vector2 translate);
	static AffineMatrix2D MakeAffineMatrix(const AffineMatrix2D& scaleMatrix, const AffineMatrix2D& rotationMatrix, const AffineMatrix2D& trancelateMatrix);
	static AffineMatrix2D MakeAffine(const Vector2& scale, float theta, const Vector2& trancelate);
};
