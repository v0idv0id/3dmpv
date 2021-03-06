#version 330 core

// GNU GPLv3
// Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com

in vec3 TexCoords;

uniform sampler2D texture1;

void main() {
  // Quick and dirty vignette

  float x, y;
  x = fract(TexCoords.x / TexCoords.z * 4.0);
  y = fract(TexCoords.y / TexCoords.z * 4.0);

  if (x > 0.99 || y > 0.99 || x < 0.01 || y < 0.01) {
    gl_FragColor = vec4(1, 0, 1, 1);
  } else if (x > 0.5 && x < 0.51) {
    gl_FragColor = vec4(0, 0, 1, 1);
  } else if (y > 0.5 && y < 0.51) {
    gl_FragColor = vec4(0, 1, 1, 1);
  } else {
    gl_FragColor = texture(texture1, TexCoords.xy / TexCoords.z);
  }
}