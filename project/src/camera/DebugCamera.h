#pragma once

#include "Matrix4x4.hpp"

// ================= temp include =========================
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// ========================================================


class DebugCamera {
private:
	Matrix4x4 matRot_ = Matrix4x4::Identity();
	Vector3 translation_ = { 0,0,0 };
	Matrix4x4 viewMatrix_ = Matrix4x4::Identity();
	Matrix4x4 projectionMatrix_ = Matrix4x4::Identity();

	bool isControlEnabled_ = false;

	// ---------------- Camera shake ----------------
	float shakeTimer_ = 0.0f;
	float shakeDuration_ = 0.0f;
	float shakeMagnitude_ = 0.0f;
	Vector3 shakeOffset_ = { 0,0,0 };

public:

	void SetIsControlEnable(bool flag) { isControlEnabled_ = flag; };

	void Initialize(int kClientWidth, int kClientHeight, Transform transform);

	void Update(BYTE key[256]);

	// Starts a short camera shake (e.g. when the player shoots). Safe to call
	// repeatedly; restarts the shake with the given parameters.
	void Shake(float duration, float magnitude) {
		shakeTimer_ = duration;
		shakeDuration_ = duration;
		shakeMagnitude_ = magnitude;
	}

	Matrix4x4 getViewProjectionMatrix() const {
		Matrix4x4 shakeTranslate = Matrix4x4::MakeTranslateMatrix(shakeOffset_);
		return shakeTranslate * viewMatrix_ * projectionMatrix_;
	}


	Matrix4x4 GetBillboardMatrix() const {
		return matRot_;
	}

	void SetTranslation(const Vector3& translation) {
		translation_ = translation;
	}

	Vector3* GetTranslation() {
		return &translation_;
	}
};