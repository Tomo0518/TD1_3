#include "ParticleRegistry.h"

std::vector<ParticleTypeInfo> ParticleRegistry::particleTypes_;

void ParticleRegistry::Initialize() {
	particleTypes_.clear();

	// ==================================
	// Combat Effects（戦闘エフェクト）
	// ==================================
	particleTypes_.push_back({
		ParticleType::Explosion,
		"Explosion",                    // JSON キー名と一致させる
		"Combat",
		TextureId::Particle_Explosion,
	0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::Hit,
		"Hit",
		"Combat",
		TextureId::Particle_Hit,
	0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::MuzzleFlash,
		"MuzzleFlash",
		"Combat",
		TextureId::Particle_Hit,
		0x555555FF                      // 白色
		});

	// ==================================
	// Environment Effects（環境エフェクト）
	// ==================================
	particleTypes_.push_back({
		ParticleType::Rain,
		"Rain",
		"Environment",
		TextureId::Particle_Rain,
	 0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::Snow,
		"Snow",
		"Environment",
		TextureId::Particle_Snow,
	 0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::Orb,
		"Orb",
		"Environment",
		TextureId::Particle_Orb,
	  0x555555FF
		});

	// ==================================
	// General Effects（汎用エフェクト）
	// ==================================
	particleTypes_.push_back({
		ParticleType::Debris,
		"Debris",
		"General",
		TextureId::Particle_Debris,
		0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::Dust,
		"Dust",
		"General",
		TextureId::Particle_Dust,
	   0x555555FF
		});

	// ==================================
	// Visual Effects（視覚エフェクト）
	// ==================================
	particleTypes_.push_back({
		ParticleType::Glow,
		"Glow",
		"Visual",
		TextureId::Particle_Glow,
	   0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::Shockwave,
		"Shockwave",
		"Visual",
		TextureId::Particle_Ring,
	   0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::Sparkle,
		"Sparkle",
		"Visual",
		TextureId::Particle_Sparkle,
	   0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::Slash,
		"Slash",
		"Visual",
		TextureId::Particle_Scratch,
	   0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::SmokeCloud,
		"SmokeCloud",
		"Visual",
		TextureId::Particle_Smoke,
	   0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::Charge,
		"Charge",
		"Visual",
		TextureId::Particle_Glow,
	   0x555555FF
		});

	particleTypes_.push_back({
		ParticleType::DigitalSpark,
		"DigitalSpark",
		"Visual",
		TextureId::Particle_Hit,
		0x555555FF
		});
}

const std::vector<ParticleTypeInfo>& ParticleRegistry::GetAllParticleTypes() {
	return particleTypes_;
}

const ParticleTypeInfo* ParticleRegistry::GetParticleType(ParticleType type) {
	for (const auto& info : particleTypes_) {
		if (info.type == type) {
			return &info;
		}
	}
	return nullptr;
}

ParticleType ParticleRegistry::GetParticleTypeByName(const std::string& name) {
	for (const auto& info : particleTypes_) {
		if (info.name == name) {
			return info.type;
		}
	}
	// デフォルト値（見つからない場合）
	return ParticleType::Explosion;
}