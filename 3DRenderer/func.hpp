#pragma once
#include "SFML/Graphics.hpp"
#include "math.h"
#include <iostream>

sf::Vector3f operator- (const sf::Vector3f& l, const sf::Vector3f& r);


namespace func {
	float z_rot(float mat[4][4]);
	float x_rot(float mat[4][4]);
	float y_rot(float mat[4][4]);

	sf::Vector3f getIntersection(sf::Vector3f plane_point, sf::Vector3f plane_normal, sf::Vector3f p0, sf::Vector3f p1);

	void mult4x4(const sf::Vector3f vec, const float matrix[4][4], sf::Vector3f& result);

	float edge_f(const sf::Vector2f& pixel, const sf::Vector3f& v0, const sf::Vector3f& v1);

	void vec_mult(const float left_mat[4][4], const float right_mat[4][4], float(&res)[4][4]);

	sf::Vector3f norm3f(const sf::Vector3f& in);
	sf::Vector3f crossv(const sf::Vector3f l, const sf::Vector3f r);
	float dotpro(const sf::Vector3f l, const sf::Vector3f r);
	void print(const sf::Vector3f& i);
}



struct Triangle
{
	sf::Vector3f verts[3];
	double area;

	//bounding box
	int b_left;
	int b_top;
	int b_right;
	int b_bot;

	Triangle(sf::Vector3f v0, sf::Vector3f v1, sf::Vector3f v2)
	{
		verts[0] = v0; verts[1] = v1; verts[2] = v2;
	}
};

