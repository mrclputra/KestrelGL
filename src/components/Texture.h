#pragma once

#include <glad/glad.h>
//#include "stb_image.h"
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

	Texture(const std::string& path, Type type = Type::ALBEDO);
	~Texture();

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	unsigned int id = 0;
	Type type;
	std::string path;

private:
	void loadFromFile(const std::string& path);
};