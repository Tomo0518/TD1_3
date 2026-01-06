#include "Affine2D.h"
#include <cmath>

AffineMatrix2D AffineMatrix2D::MakeScaleMatrix(const Vector2 scale) {
	AffineMatrix2D matrix;

	matrix.m[0][0] = scale.x;
	matrix.m[0][1] = 0.0f;
	matrix.m[0][2] = 0.0f;

	matrix.m[1][0] = 0.0f;
	matrix.m[1][1] = scale.y;
	matrix.m[1][2] = 0.0f;

	matrix.m[2][0] = 0.0f;
	matrix.m[2][1] = 0.0f;
	matrix.m[2][2] = 1.0f;

	return matrix;
}

AffineMatrix2D AffineMatrix2D::MakeRotationMatrix(float theta) {
	AffineMatrix2D matrix;

	matrix.m[0][0] = cosf(theta);
	matrix.m[0][1] = sinf(theta);
	matrix.m[0][2] = 0.0f;

	matrix.m[1][0] = -sinf(theta);
	matrix.m[1][1] = cosf(theta);
	matrix.m[1][2] = 0.0f;

	matrix.m[2][0] = 0.0f;
	matrix.m[2][1] = 0.0f;
	matrix.m[2][2] = 1.0f;

	return matrix;
}

AffineMatrix2D AffineMatrix2D::MakeTranslateMatrix(const Vector2 translate) {
	AffineMatrix2D matrix;

	matrix.m[0][0] = 1.0f;
	matrix.m[0][1] = 0.0f;
	matrix.m[0][2] = 0.0f;

	matrix.m[1][0] = 0.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[1][2] = 0.0f;

	matrix.m[2][0] = translate.x;
	matrix.m[2][1] = translate.y;
	matrix.m[2][2] = 1.0f;

	return matrix;
}

AffineMatrix2D AffineMatrix2D::MakeAffineMatrix(const AffineMatrix2D& scaleMatrix, const AffineMatrix2D& rotationMatrix, const AffineMatrix2D& translateMatrix) {
	AffineMatrix2D affinMatrix;

	affinMatrix = static_cast<AffineMatrix2D>(Matrix3x3::Multiply(scaleMatrix, rotationMatrix));
	affinMatrix = static_cast<AffineMatrix2D>(Matrix3x3::Multiply(affinMatrix, translateMatrix));

	return affinMatrix;
}

AffineMatrix2D AffineMatrix2D::MakeAffine(const Vector2& scale, float theta, const Vector2& translate) {
	AffineMatrix2D scaleMatrix = AffineMatrix2D::MakeScaleMatrix(scale);
	AffineMatrix2D rotateMatrix = AffineMatrix2D::MakeRotationMatrix(theta);
	AffineMatrix2D translateMatrix = AffineMatrix2D::MakeTranslateMatrix(translate);

	return AffineMatrix2D::MakeAffineMatrix(scaleMatrix, rotateMatrix, translateMatrix);
}