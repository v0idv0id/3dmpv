#version 330 core
in vec3 TexCoords;

uniform sampler2D texture1;

void main()
{    
    // Quick and dirty vignette
    
    vec2 st= gl_FragCoord.xy; 
    st = TexCoords.xy;
    vec4 colorx=mix(vec4(1.,1.,1.,1.),vec4(.0,.0,.0,1.0), 2 *length(st-vec2(.5,.5)));
    vec4 frag0 = texture(texture1, TexCoords.xy / TexCoords.z );
    gl_FragColor = texture(texture1, TexCoords.xy / TexCoords.z );
}