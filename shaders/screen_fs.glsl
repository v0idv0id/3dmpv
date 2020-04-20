#version 330 core

// GNU GPLv3
// Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com

in vec3 TexCoords;

uniform sampler2D texture1;
uniform float vignette;

void main() {
  // Quick and dirty vignette
  vec2 st = TexCoords.xy / TexCoords.z;
  vec4 vig = mix(vec4(1., 1., 1., 1.), vec4(0.0, 0.0, 0.0, 1.0),
                      vignette * length(st - vec2(.5, .5)));

  gl_FragColor = texture(texture1, TexCoords.xy / TexCoords.z) * vig;
}