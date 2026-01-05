#version 460 core
out vec4 FragColor;
in vec3 LocalPos;

uniform samplerCube skybox;

void main() {    
    // note, input skybox map is not tonemapped
    vec3 sampleDir = vec3(LocalPos.x, -LocalPos.y, LocalPos.z);
    vec3 envColor = texture(skybox, sampleDir).rgb;

    // clamp
    envColor = clamp(envColor, 0.0, 1000.0);

    // compression
    // https://graphicrants.blogspot.com/2013/12/tone-mapping.html?m=1
    float luma = dot(envColor, vec3(0.2126, 0.7152, 0.0722));
    float weight = 1.0 / (1.0 + luma);
    envColor *= weight;

    // gamma correction
    envColor = pow(envColor, vec3(1.0/2.2)); 

    // output
    FragColor = vec4(envColor, 1.0);
}