#version 330 core
// GNU GPLv3
// Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com

layout(location = 0) in vec3 aPos;


void main() {
  gl_PointSize=100.;
  gl_Position = vec4(aPos, 1.0);
}