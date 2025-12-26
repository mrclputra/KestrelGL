#version 460 core
out vec4 FragColor;
in vec3 LocalPos;

uniform samplerCube skybox;

void main() {    
    vec3 envColor = texture(skybox, LocalPos).rgb;

    // tonemapping; reinhard
    envColor = envColor / (envColor + vec3(1.0));
    
    // gamma correction
    envColor = pow(envColor, vec3(1.0/2.2)); 
  
    FragColor = vec4(envColor, 1.0);
}