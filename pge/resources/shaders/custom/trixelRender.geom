#version 400

uniform mat4 pgeViewModel;
uniform mat4 pgeProjectionViewModel;
uniform mat3 pgeNormal;

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

in vec4 rotationToGeom[];
in vec3 colorToGeom[];

// For fragment shader
smooth out vec3 viewPosition;
smooth out vec3 viewNormal;
out vec3 color;

const float scale = 0.0625;

vec3 rotatePoint(vec3 point, vec4 quaternion) {
	vec3 quatVec = quaternion.xyz;
	vec3 uv = cross(quatVec, point);
	vec3 uuv = cross(quatVec, uv);
	uv *= 2.0 * quaternion.w;
	uuv *= 2.0;

	return point + uv + uuv;
}

void main() {
	const vec3 corners[8] = vec3[8](
		vec3(-0.5, 0.5, -0.5), // Front top left
		vec3( 0.5, 0.5, -0.5), // Front top right
		vec3( 0.5, -0.5, -0.5), // Front bottom right
		vec3(-0.5, -0.5, -0.5), // Front bottom left
		vec3(-0.5, 0.5, 0.5), // Back top left
		vec3( 0.5, 0.5, 0.5), // Back top right
		vec3( 0.5, -0.5, 0.5), // Back bottom right
		vec3(-0.5, -0.5, 0.5)  // Back bottom left
	);

	const vec3 cubeNormals[6] = vec3[6] (
		vec3(1.0, 0.0, 0.0),
		vec3(-1.0, 0.0, 0.0),
		vec3(0.0, 0.0, 1.0),
		vec3(0.0, 0.0, -1.0),
		vec3(0.0, 1.0, 0.0),
		vec3(0.0, -1.0, 0.0)
	);

	color = colorToGeom[0];

	vec3 world0 = rotatePoint(corners[0], rotationToGeom[0]) * scale + gl_in[0].gl_Position.xyz;
	vec3 world1 = rotatePoint(corners[1], rotationToGeom[0]) * scale + gl_in[0].gl_Position.xyz;
	vec3 world2 = rotatePoint(corners[2], rotationToGeom[0]) * scale + gl_in[0].gl_Position.xyz;
	vec3 world3 = rotatePoint(corners[3], rotationToGeom[0]) * scale + gl_in[0].gl_Position.xyz;
	vec3 world4 = rotatePoint(corners[4], rotationToGeom[0]) * scale + gl_in[0].gl_Position.xyz;
	vec3 world5 = rotatePoint(corners[5], rotationToGeom[0]) * scale + gl_in[0].gl_Position.xyz;
	vec3 world6 = rotatePoint(corners[6], rotationToGeom[0]) * scale + gl_in[0].gl_Position.xyz;
	vec3 world7 = rotatePoint(corners[7], rotationToGeom[0]) * scale + gl_in[0].gl_Position.xyz;

	vec4 corner0 = pgeProjectionViewModel * vec4(world0, 1.0);
    vec4 corner1 = pgeProjectionViewModel * vec4(world1, 1.0);
    vec4 corner2 = pgeProjectionViewModel * vec4(world2, 1.0);
    vec4 corner3 = pgeProjectionViewModel * vec4(world3, 1.0);
    vec4 corner4 = pgeProjectionViewModel * vec4(world4, 1.0);
    vec4 corner5 = pgeProjectionViewModel * vec4(world5, 1.0);
    vec4 corner6 = pgeProjectionViewModel * vec4(world6, 1.0);
    vec4 corner7 = pgeProjectionViewModel * vec4(world7, 1.0);

	vec3 view0 = (pgeViewModel * vec4(world0, 1.0)).xyz;
    vec3 view1 = (pgeViewModel * vec4(world1, 1.0)).xyz;
    vec3 view2 = (pgeViewModel * vec4(world2, 1.0)).xyz;
    vec3 view3 = (pgeViewModel * vec4(world3, 1.0)).xyz;
    vec3 view4 = (pgeViewModel * vec4(world4, 1.0)).xyz;
    vec3 view5 = (pgeViewModel * vec4(world5, 1.0)).xyz;
    vec3 view6 = (pgeViewModel * vec4(world6, 1.0)).xyz;
    vec3 view7 = (pgeViewModel * vec4(world7, 1.0)).xyz;
 
    viewNormal = pgeNormal * rotatePoint(cubeNormals[0], rotationToGeom[0]);
	viewPosition = view0;
    gl_Position = corner0;
    EmitVertex();
	viewPosition = view1;
    gl_Position = corner1;
    EmitVertex();
	viewPosition = view3;
    gl_Position = corner3;
    EmitVertex();
	viewPosition = view2;
    gl_Position = corner2;
    EmitVertex();
    EndPrimitive();
    
	viewNormal = pgeNormal * rotatePoint(cubeNormals[1], rotationToGeom[0]);
	viewPosition = view5;
    gl_Position = corner5;
    EmitVertex();
	viewPosition = view4;
    gl_Position = corner4;
    EmitVertex();
	viewPosition = view6;
    gl_Position = corner6;
    EmitVertex();
	viewPosition = view7;
    gl_Position = corner7;
    EmitVertex();
    EndPrimitive();
    
    viewNormal = pgeNormal * rotatePoint(cubeNormals[2], rotationToGeom[0]);
	viewPosition = view4;
    gl_Position = corner4;
    EmitVertex();
	viewPosition = view0;
    gl_Position = corner0;
    EmitVertex();
	viewPosition = view7;
    gl_Position = corner7;
    EmitVertex();
	viewPosition = view3;
    gl_Position = corner3;
    EmitVertex();
    EndPrimitive();
    
    viewNormal = pgeNormal * rotatePoint(cubeNormals[3], rotationToGeom[0]);
	viewPosition = view1;
    gl_Position = corner1;
    EmitVertex();
	viewPosition = view5;
    gl_Position = corner5;
    EmitVertex();
	viewPosition = view2;
    gl_Position = corner2;
    EmitVertex();
	viewPosition = view6;
    gl_Position = corner6;
    EmitVertex();
    EndPrimitive();
    
    viewNormal = pgeNormal * rotatePoint(cubeNormals[4], rotationToGeom[0]);
	viewPosition = view4;
    gl_Position = corner4;
    EmitVertex();
	viewPosition = view5;
    gl_Position = corner5;
    EmitVertex();
	viewPosition = view0;
    gl_Position = corner0;
    EmitVertex();
	viewPosition = view1;
    gl_Position = corner1;
    EmitVertex();
    EndPrimitive();
    
    viewNormal = pgeNormal * rotatePoint(cubeNormals[5], rotationToGeom[0]);
	viewPosition = view3;
    gl_Position = corner3;
    EmitVertex();
	viewPosition = view2;
    gl_Position = corner2;
    EmitVertex();
	viewPosition = view7;
    gl_Position = corner7;
    EmitVertex();
	viewPosition = view6;
    gl_Position = corner6;
    EmitVertex();
    EndPrimitive();   
}
