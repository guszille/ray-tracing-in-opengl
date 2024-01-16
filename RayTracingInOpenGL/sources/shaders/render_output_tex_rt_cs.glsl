#version 460 core

#define NOL 1 // Number of lights.
#define NOS 1 // Number of spheres.

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D u_image_output;

struct Light
{
	vec3 position;
	vec3 color;

	float intensity;
};

struct Material
{
	vec3 diffuse_color;
};

struct Sphere
{
	vec3 center;

	float radius;

	Material material;
};

struct Plane
{
	float y_position;

	vec3 normal;

	float x_size;
	float z_size;

	Material material;
};

struct Hit
{
	vec3 point;
	vec3 normal;

	Material material;

	bool performed;
};

uniform vec3 u_view_position;
uniform mat4 u_view_matrix;
uniform vec3 u_backgrounf_color = vec3(0.2, 0.4, 0.8);

uniform float u_fov = radians(45.0);
uniform float u_global_threshold = 1e-3;

Light lights[NOL];
Sphere spheres[NOS];
Plane plane;

float ray_sphere_intersect(vec3 origin, vec3 direction, Sphere sphere)
{
	vec3 xa = origin - sphere.center;
	float b = dot(xa, direction);
	float delta = (b * b) - dot(xa, xa) + (sphere.radius * sphere.radius);

	if (delta < 0) return -1.0;

	float s1 = -b - sqrt(delta);
	float s2 = -b + sqrt(delta);

	if (s1 > 0) return s1;
	else if (s2 > 0) return s2;

	return -1.0;
}

Hit scene_intersect(vec3 origin, vec3 direction)
{
	Hit hit_info;

	hit_info.performed = false;

	float closest_sphere_distance = 1e32;

	for (int i = 0; i < NOS; i++)
	{
		float sphere_distance = ray_sphere_intersect(origin, direction, spheres[i]);

		if (sphere_distance > 0.0 && sphere_distance < closest_sphere_distance)
		{
			closest_sphere_distance = sphere_distance;

			hit_info.point = origin + (direction * sphere_distance);
			hit_info.normal = normalize(hit_info.point - spheres[i].center);
			hit_info.material = spheres[i].material;
			hit_info.performed = true;
		}
	}

	if (abs(direction.y) > u_global_threshold)
	{
		float plane_distance = (plane.y_position - origin.y) / direction.y;

		if (plane_distance > 0.0 && plane_distance < closest_sphere_distance)
		{
			vec3 ray_plane_intersect_point = origin + (direction * plane_distance);
			
			if (abs(ray_plane_intersect_point.x) < plane.x_size && abs(ray_plane_intersect_point.z) < plane.z_size)
			{
				hit_info.point = ray_plane_intersect_point;
				hit_info.normal = plane.normal;
				hit_info.material = plane.material;
				hit_info.performed = true;
			}
		}
	}

	return hit_info;
}

vec3 cast_ray(vec3 origin, vec3 direction)
{
	Hit hit_info_1 = scene_intersect(origin, direction);

	if (hit_info_1.performed)
	{
		vec3 light_diffuse_comp = vec3(1.0, 1.0, 1.0);
		float light_diffuse_factor = 0.0;

		for (int i = 0; i < NOL; i++)
		{
			vec3 light_direction = normalize(lights[i].position - hit_info_1.point);
			vec3 new_origin = dot(light_direction, hit_info_1.normal) < 0.0 ? hit_info_1.point - (hit_info_1.normal * u_global_threshold) : hit_info_1.point + (hit_info_1.normal * u_global_threshold);

			Hit hit_info_2 = scene_intersect(new_origin, light_direction);

			if (hit_info_2.performed)
			{
				continue;
			}

			light_diffuse_comp *= lights[i].color;
			light_diffuse_factor += lights[i].intensity * clamp(dot(light_direction, hit_info_1.normal), 0.0, 1.0);
		}

		return (hit_info_1.material.diffuse_color * light_diffuse_comp) * light_diffuse_factor;
	}

	return u_backgrounf_color;
}

void main()
{
	// Lights.
	lights[0].position = vec3(5.0, 5.0, 5.0);
	lights[0].color = vec3(1.0, 1.0, 1.0);
	lights[0].intensity = 1.5;

	// Spheres.
	spheres[0].center = vec3(0.0, 0.0, 0.0);
	spheres[0].radius = 1.0;
	spheres[0].material.diffuse_color = vec3(0.75, 0.15, 0.75);

	// Plane.
	plane.y_position = -2.0; // The plane equation is "y = -2".
	plane.normal = vec3(0.0, 1.0, 0.0);
	plane.x_size = 10.0;
	plane.z_size = 10.0;
	plane.material.diffuse_color = vec3(0.4, 0.8, 0.4);

	// Shader and image properties.
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 image_dims = imageSize(u_image_output); // Fetch image dimensions.

	float x = (2.0 * (pixel_coords.x + 0.5) / image_dims.x - 1) * tan(u_fov / 2.0) * image_dims.x / image_dims.y;
	float y = (2.0 * (pixel_coords.y + 0.5) / image_dims.y - 1) * tan(u_fov / 2.0);

	vec3 view_direction = inverse(mat3(u_view_matrix)) * normalize(vec3(x, y, -1.0));
	vec4 pixel = vec4(cast_ray(u_view_position, view_direction), 1.0);

	imageStore(u_image_output, pixel_coords, pixel);
}
