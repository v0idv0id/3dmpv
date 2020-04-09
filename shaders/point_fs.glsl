#version 330 core

void main() {
  // Very simple GL_POINTS (which are squares) to circle shader.
  float diameter=0.8; //1.0 is max
  if (length(gl_PointCoord - vec2(0.5)) > diameter/2)
    discard;
  gl_FragColor = vec4(1.0, 0.0, 1.0, 0.1);
}