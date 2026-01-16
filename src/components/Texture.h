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
	//Texture() = default;
	Texture(const std::string& path, Type type = Type::ALBEDO) : path(path), type(type) {}
	//Texture(const std::string path, Type type = Type::ALBEDO) {
	//	this->type = type;
	//	this->path = path;
	//}
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

	std::string path;
	unsigned int id = 0;
	Type type;
};