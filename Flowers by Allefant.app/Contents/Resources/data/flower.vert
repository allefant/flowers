attribute vec4 al_pos;
attribute vec3 al_user_attr_0; // normal
attribute vec4 al_color;
attribute vec2 al_texcoord;
uniform mat4 al_projview_matrix;
uniform bool al_use_tex_matrix;
uniform mat4 al_tex_matrix;
uniform vec3 light_direction;
varying vec4 varying_color;
varying vec2 varying_texcoord;
varying float shade;
varying float fog;
void main() {
    varying_color = al_color;
    if (al_use_tex_matrix) {
        vec4 uv = al_tex_matrix * vec4(al_texcoord, 0, 1);
        varying_texcoord = vec2(uv.x, uv.y);
    }
    else {
        varying_texcoord = al_texcoord;
    }
    float d = dot(al_user_attr_0, light_direction);
    float d1 = (1.0 + d) / 2.0;
    float d2 = (1.0 - d) / 4.0;
    vec4 p = al_projview_matrix * al_pos;
    shade = d1 + d2;
    fog = clamp((p.z - 1000.0) / 1000.0, 0.0, 1.0);
    gl_Position = p;
}
