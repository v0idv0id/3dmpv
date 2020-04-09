#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aTexCoords;
layout (location = 2) in vec3 aTexCoords_orig;

out vec3 TexCoords;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords; 
    // We only need the "model" part of M*P*V here since we are only a flat "screen"   
    // gl_Position =   model *  vec4(aPos, 1.0);
    gl_Position =   vec4(aPos, 1.0);

}