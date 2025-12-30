#version 460 core
out vec4 FragColor;
in vec3 LocalPos;

uniform samplerCube skybox;

// ACES filmic
// https://en.wikipedia.org/wiki/Academy_Color_Encoding_System
vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {    
    // if the input HDRI is already tonemapped,
    // then there is no need to apply Reinhard and Gamma Correction
    // this should be the case for most jpeg files

    // todo: tonemapping and gamma correction should be moved to the conversion shader
    //  but only in the event that this is what we want; spherical harmonics not using original data

    vec3 envColor = texture(skybox, vec3(LocalPos.x, -LocalPos.y, LocalPos.z)).rgb;

    // tonemapping; aces
    envColor = aces(envColor);

    // gamma correction
    envColor = pow(envColor, vec3(1.0/2.2)); 

    // output
    FragColor = vec4(envColor, 1.0);
}