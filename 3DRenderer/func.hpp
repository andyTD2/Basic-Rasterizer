#pragma once
#include "SFML/Graphics.hpp"
#include "math.h"
#include <iostream>
#include "immintrin.h"

namespace func {
	sf::Vector3f getIntersection(sf::Vector3f plane_point, sf::Vector3f plane_normal, sf::Vector3f p0, sf::Vector3f p1, float& t);

	void vecXmatrix(const sf::Vector3f vec, const float matrix[4][4], sf::Vector3f& result);

	float edge_f(const sf::Vector2f& pixel, const sf::Vector3f& v0, const sf::Vector3f& v1);

	void matrixXmatrix(const float left_mat[4][4], const float right_mat[4][4], float(&res)[4][4]);

	sf::Vector3f norm3f(const sf::Vector3f& in);
	sf::Vector3f crossv(const sf::Vector3f l, const sf::Vector3f r);
	float dotpro(const sf::Vector3f l, const sf::Vector3f r);
	sf::Vector3f vec3XScalar(const sf::Vector3f l, float r);
	sf::Vector2f vec2XScalar(const sf::Vector2f l, float r);
	void print(const sf::Vector3f& i);
}
