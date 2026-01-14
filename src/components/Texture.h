#pragma once

#include <glad/glad.h>
#include <string>

#include <logger.h>

class Texture {
public:
	enum class Type {
		ALBEDO,
		METALLIC_ROUGHNESS,
		NORMAL,
		OCCLUSION,
		EMISSION
	};

	// constructor
	Texture(Type type = Type::ALBEDO) {
		this->type = type;
	}
	~Texture() {
		if (id != 0) {
			glDeleteTextures(1, &id);
		}
	}

	void bind(unsigned int slot = 0) const {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, id);
	}
	void unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	unsigned int id = 0;
	Type type;
};