#pragma once
#include "SFML/Graphics.hpp"
#include "math.h"
#include <iostream>
#include "immintrin.h"

class vec4
{
public:
	float x, y, z, w;

	vec4();
	vec4(float newX, float newY, float newZ);
	vec4(const vec4& other);
	vec4 operator+(const vec4& other);
	vec4& operator+=(const vec4& other);
	vec4 operator-(const vec4& other) const;
	vec4& operator-=(const vec4& other);
	vec4 operator*(float r);
};

class vec2
{
public:
	float x, y;
	vec2();
	vec2(float newX, float newY);
};

namespace func {
	vec4 getIntersection(vec4 plane_point, vec4 plane_normal, vec4 p0, vec4 p1, float& t);

	float vecXmatrix(const vec4& vec, const float matrix[4][4], vec4& result, bool project);

	float edge_f(const vec2& pixel, const vec4& v0, const vec4& v1);

	void matrixXmatrix(const float left_mat[4][4], const float right_mat[4][4], float(&res)[4][4]);

	vec4 norm(const vec4& in);
	vec4 crossProd(const vec4& l, const vec4& r);
	float dotPro(const vec4& l, const vec4& r);
	sf::Vector3f vec3XScalar(const sf::Vector3f l, float r);
	sf::Vector2f vec2XScalar(const sf::Vector2f l, float r);
	void print(const vec2& i);
	void print(const vec4& i);
	float distance(const vec4& p1, const vec4& p2);
	float dotpro2f(const vec2& l, const vec2& r);

	float getDist(const vec4& planeNormal, const vec4& planePoint, const vec4& point);
}
