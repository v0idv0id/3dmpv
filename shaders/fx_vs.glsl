#version 330 core

// GNU GPLv3
// Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com


layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aTexCoords;
layout (location = 2) in vec3 aTexCoords_orig;

out vec3 TexCoords;


void main()
{
    TexCoords = aTexCoords; 
    gl_Position =   vec4(aPos, 1.0);

}