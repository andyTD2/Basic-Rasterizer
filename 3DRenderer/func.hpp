#pragma once
#include "SFML/Graphics.hpp"
#include "immintrin.h"

class vec4
{
public:
	float x, y, z, w;

	vec4();
	vec4(float newX, float newY, float newZ);
	vec4(const vec4& other);
	vec4 operator+(const vec4& other) const ;
	vec4& operator+=(const vec4& other);
	vec4 operator-(const vec4& other) const;
	vec4& operator-=(const vec4& other);
	vec4 operator*(float r) const;
};

class vec2
{
public:
	float x, y;
	vec2();
	vec2(float newX, float newY);
};

namespace func {
	/**
	 * @brief Retrieves the point at which a line defined by p0 -> p1 intersects a plane.
	 *
	 * @param planePoint: any point on the plane that is tested against
	 * @param panForward: a normal to the plane that is tested against
	 * @param p0: endpoint of the line to be tested
	 * @param p1: other endpoint of the line to be tested
	 * @param t: holds the normalized distance(0 - 1.0) to the intersection point.
	 *
	 * @return vec4 the point at which the line intersects the plane
	 */
	vec4 getIntersection(const vec4& planePoint, const vec4& planeNormal, const vec4& p0, const vec4& p1, float& t);

	/**
	 * @brief Given a line defined by v0->v1, tests whether a point is on the right or left side of it.
	 *
	 * By testing against the 3 lines of a triangle, we can find out whether or not a point lines within
	 * a triangle.
	 * 
	 * @param pixel: the point or pixel that we test
	 * @param v0: an endpoint of the line to be tested against
	 * @param v1 the other endpoint of the line to be tested against
	 *
	 * @return float a distance value from the line. Larger values are further away. The sign indicates which side
	 * the point is on relative to the line.
	 */
	float edgeFunc(const vec2& pixel, const vec4& v0, const vec4& v1);

	//Affine version does not divide by w, whereas projective version does
	//Separate functions so that we don't have to perform any checks to determine which version to use
	void vecXmatrixAffine(const vec4& vec, const float matrix[4][4], vec4& result);
	void vecXmatrixProjective(const vec4& vec, const float matrix[4][4], vec4& result);

	//Various vector math functions
	void matrixXmatrix(const float left_mat[4][4], const float right_mat[4][4], float(&res)[4][4]);
	vec4 norm(const vec4& in);
	vec4 crossProd(const vec4& l, const vec4& r);
	float dotPro(const vec4& l, const vec4& r);
	sf::Vector3f vec3XScalar(const sf::Vector3f l, float r);
	sf::Vector2f vec2XScalar(const sf::Vector2f l, float r);
	float dotpro2f(const vec2& l, const vec2& r);
	float getDist(const vec4& planeNormal, const vec4& planePoint, const vec4& point);
}