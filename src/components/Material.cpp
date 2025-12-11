#include "Material.h"
#include <string>

void Material::apply(Shader& shader) const {
	// TODO: apply textures here

	// apply albedo color
	shader.setVec3("albedo", albedo); // !!!
}