#ifdef GL_ES
precision lowp float;
#endif

uniform sampler2D al_tex;
uniform bool al_use_tex;
uniform bool al_alpha_test;
uniform int al_alpha_func;
uniform float al_alpha_test_val;
varying vec4 varying_color;
varying vec2 varying_texcoord;
varying float shade;
varying float fog;

bool alpha_test_func(float x, int op, float compare);

void main() {
    vec4 c;
    if (al_use_tex)
        c = varying_color * texture2D(al_tex, varying_texcoord);
    else
        c = varying_color;
    c.r *= shade;
    c.g *= shade;
    c.b *= shade;
    c.r = 0.7 * fog + c.r * (1.0 - fog);
    c.g = 0.8 * fog + c.g * (1.0 - fog);
    c.b = 0.9 * fog + c.b * (1.0 - fog);
    
    gl_FragColor = c;
}
