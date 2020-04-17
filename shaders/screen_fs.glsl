#version 330 core
in vec3 TexCoords;

uniform sampler2D texture1;

void main() {
  // Quick and dirty vignette

  float x, y;
  x = fract(TexCoords.x/ TexCoords.z * 4.0);
  y = fract(TexCoords.y/ TexCoords.z * 4.0);

  // vec2 st= gl_FragCoord.xy;
  vec2 st = TexCoords.xy / TexCoords.z;
  vec4 vignette = mix(vec4(1., 1., 1., 1.), vec4(.0, .0, .0, 1.0),
                    2 * length(st - vec2(.5, .5)));
  // vec4 frag0 = texture(texture1, TexCoords.xy / TexCoords.z );
  if (x > 0.99 || y > 0.99 || x < 0.01 || y < 0.01) {
    gl_FragColor = vec4(1, 0, 1, 1);
  }  else if(x > 0.5 && x < 0.51) {
    gl_FragColor = vec4(0, 0, 1, 1);
  } else if(y > 0.5 && y < 0.51) {
    gl_FragColor = vec4(0, 1, 1, 1);

  }
  
  
  else {
    gl_FragColor = texture(texture1, TexCoords.xy / TexCoords.z) * vignette;
  }

}