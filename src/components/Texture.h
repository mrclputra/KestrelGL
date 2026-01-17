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
	Texture(const Type type, const std::string path) : type(type), path(path) {}
	~Texture() {
		if (id != 0) {
			glDeleteTextures(1, &id);
		}
	}

	void bind(unsigned int slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, id);
	}
	void unbind() {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// attributes
	unsigned int id = 0; // handler to GPU
	const Type type;
	const std::string path;
};