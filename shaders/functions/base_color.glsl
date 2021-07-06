#include "../expressions/eye_refraction.glsl"
#include "../expressions/sphere_mask.glsl"

vec3 getBaseColor(vec2 uv,
                  vec3 normal,
                  mat3 TBN,
                  vec3 view_dir) {
    float depth_plane_offset;
    {
        vec2 displacement_uv = vec2(scale_by_center * iris_uv_radius + 0.5, 0.5);
        depth_plane_offset = texture(eye_midplane_displacement, displacement_uv).r;
    }

    vec3 eye_direction_world;
    {
        vec3 eye_n = texture(wizard_eye_n, uv).rgb;

        eye_n = eye_n * 2.0 - 1.0;
        eye_n = normalize(TBN * eye_n);

        eye_direction_world = eye_n;
    }

    eye_refraction_result r = eye_refraction(
            uv,
            TBN,
            texture(eye_midplane_displacement, uv).rgb,
            depth_plane_offset,
            eye_direction_world,
            normal,
            view_dir,
            vec2(limbus_uv_width_color, limbus_uv_width_shading)
    );

    vec2 refracted_uv = r.refracted_uv;
    vec2 iris_mask = r.iris_mask;

    vec3 base_color;
    {
        vec2 uv_scaled = scale_uv_by_center(uv, scale_by_center);
        vec2 uv_with_refraction = mix(uv_scaled, refracted_uv, refraction_on_off);

        vec2 centered_uv = (uv_with_refraction - 0.5) * (1.0 / (iris_uv_radius * 2.0)) + 0.5;

        vec2 scale_pulpis;
        {
            vec2 uv_centered = centered_uv - vec2(0.5, 0.5);
            float uv_length = length(uv_centered);
            vec2 uv_max = normalize(uv_centered) * 0.5;
            vec2 UVscaled = mix(uv_max, vec2(0.0, 0.0), clamp((1.0 - uv_length * 2.0) * pupil_scale, 0.0, 1.0));
            scale_pulpis = UVscaled + vec2(0.5, 0.5);
        }

        vec3 sclera_color = sclera_brightness * texture(eye_sclera_base_color, uv_scaled).rgb;
        vec3 iris_color = iris_brightness * texture(eye_iris_base_color, scale_pulpis).rgb;
        iris_color *= (1.0 - pow(length((scale_pulpis - 0.5) * limbus_dark_scale), limbus_pow));

        vec3 composited_base_color = mix(sclera_color, iris_color, iris_mask.r);

        vec3 iris_masked_color = sphere_mask(scale_pulpis, vec2(0.5), 0.18, 0.2) * cloudy_iris;

        vec3 c1 = iris_masked_color + composited_base_color;
        vec3 c2 = mix(eye_corner_darkness_color, vec3(1.0), sphere_mask(uv_scaled, vec2(0.5), shadow_radius, shadow_hardness));

        base_color = c1 * c2;
    }

    return base_color;
}