float fresnel(vec3 normal, vec3 view, float power) {
    return pow((1.0 - clamp(dot(normalize(normal), normalize(view)), 0.0, 1.0)), power);
}

float fresnel(float exponent, float base_reflect, vec3 normal, vec3 view_dir) {
    return pow(1.0 - max(0.0, dot(normal, view_dir)), exponent) * (1.0 - base_reflect) + base_reflect;
}
