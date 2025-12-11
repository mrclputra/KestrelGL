#include "Material.h"
#include <string>

void Material::apply(Shader& shader) const {
	// TODO: apply textures here when I get to implementing it

	// apply components to shader
	shader.setVec3("albedo", albedo); // !!!
}