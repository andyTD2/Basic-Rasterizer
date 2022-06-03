#pragma once
#include "Rasterizer.hpp"
#include <iostream>



Rasterizer::Rasterizer(int _w_width, int _w_height, float _c_near, float _c_far, int _fov) :
	w_width(_w_width), w_height(_w_height), c_near(_c_near), c_far(_c_far), fov(_fov)
{
	const float pi = 3.141592653;
	aspect_ratio = w_width / w_height;

	//canvas size and dimensions
	c_width = tan((fov / 2) * pi / 180) * c_near * 2;
	c_height = c_width;
	c_top = c_height / 2;
	c_right = c_width / 2;
	c_bot = -c_top;
	c_left = -c_right;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			p_mat[i][j] = 0;
		}
	}

	//projection matrix
	p_mat[0][0] = aspect_ratio * 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	p_mat[1][1] = 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	p_mat[2][2] = c_far / (c_far - c_near);
	p_mat[2][3] = 1;
	p_mat[3][2] = (-c_far * c_near) / (c_far - c_near);
	p_mat[3][3] = 0;

}

bool Rasterizer::project_triangle(Triangle& tri, float mat[4][4])
{
	func::vecXmatrix(tri.verts[0], mat, tri.projVerts[0]);
	func::vecXmatrix(tri.verts[1], mat, tri.projVerts[1]);
	func::vecXmatrix(tri.verts[2], mat, tri.projVerts[2]);

	float bLeft = std::min({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x });
	float bTop = std::min({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y });
	float bRight = std::max({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x });
	float bBot = std::max({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y });

	if (bLeft < 1.0 && bRight > -1 && bTop < 1 && bBot > -1)
	{
		for (int i = 0; i < 3; ++i)
		{
			tri.projVerts[i].x = floor((tri.projVerts[i].x + 1) / 2 * w_width);
			tri.projVerts[i].y = floor((1 - tri.projVerts[i].y) / 2 * w_height);
		}
		return true;
	}
	return false;
}

void Rasterizer::rot_x(Triangle& tri, float degrees, sf::Vector3f(&trans_verts)[3])
{
	float x_mat[4][4] = {
		{1.0, 0.0, 0.0, 0.0},
		{0.0, cos(degrees * 3.14159265359 / 180), sin(degrees * 3.14159265359 / 180), 0.0},
		{0.0, -sin(degrees * 3.14159265359 / 180), cos(degrees * 3.14159265359 / 180), 0.0},
		{0.0, 0.0, 0.0, 1.0}
	};

	func::vecXmatrix(tri.verts[0], x_mat, trans_verts[0]);
	func::vecXmatrix(tri.verts[1], x_mat, trans_verts[1]);
	func::vecXmatrix(tri.verts[2], x_mat, trans_verts[2]);

	return;
}

void Rasterizer::rot_y(Triangle& tri, float degrees, sf::Vector3f(&trans_verts)[3])
{
	float y_mat[4][4] = {
		{cos(degrees * 3.14159265359 / 180), 0.0, -sin(degrees * 3.14159265359 / 180), 0.0},
		{0.0, 1.0, 0.0, 0.0 },
		{sin(degrees * 3.14159265359 / 180), 0.0, cos(degrees * 3.14159265359 / 180), 0.0},
		{0.0, 0.0, 0.0, 1.0}
	};


	func::vecXmatrix(tri.verts[0], y_mat, trans_verts[0]);
	func::vecXmatrix(tri.verts[1], y_mat, trans_verts[1]);
	func::vecXmatrix(tri.verts[2], y_mat, trans_verts[2]);
	return;
}
void Rasterizer::rot_z(Triangle& tri, float degrees, sf::Vector3f(&trans_verts)[3])
{
	float z_mat[4][4] = {
		{cos(degrees * 3.14159265359 / 180), sin(degrees * 3.14159265359 / 180), 0.0, 0.0},
		{-sin(degrees * 3.14159265359 / 180), cos(degrees * 3.14159265359 / 180), 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{0.0, 0.0, 0.0, 1.0}
	};


	func::vecXmatrix(tri.verts[0], z_mat, trans_verts[0]);
	func::vecXmatrix(tri.verts[1], z_mat, trans_verts[1]);
	func::vecXmatrix(tri.verts[2], z_mat, trans_verts[2]);

	return;
}

int Rasterizer::clip_triangle_near(const Triangle& tri, sf::Vector3f(&proj_verts)[3], std::vector<Triangle>& out) const
{
	//early outs

	if (proj_verts[0].z >= c_near && proj_verts[1].z >= c_near && proj_verts[2].z >= c_near)
	{

		out.push_back(Triangle(proj_verts[0], proj_verts[1], proj_verts[2], tri.tCoords[0], tri.tCoords[1], tri.tCoords[2], tri.associatedMtl));
		return 1;
	}
	else if (proj_verts[0].z < c_near && proj_verts[1].z < c_near && proj_verts[2].z < c_near)
	{
		return 0;
	}

	std::vector<sf::Vector3f> clippedVertices;
	std::vector<sf::Vector3f> safeVertices;
	std::vector<sf::Vector2f> clippedTVertices;
	std::vector<sf::Vector2f> safeTVertices;

	for (int i = 0; i < 3; ++i)
	{
		if (proj_verts[i].z < c_near)
		{
			clippedVertices.push_back(proj_verts[i]);
			clippedTVertices.push_back(tri.tCoords[i]);
		}
		else
		{
			safeVertices.push_back(proj_verts[i]);
			safeTVertices.push_back(tri.tCoords[i]);
		}
	}

	// only need to create one new triangle
	if (clippedVertices.size() == 2)
	{
		float t;
		sf::Vector3f one = func::getIntersection(sf::Vector3f(0, 0, c_near), sf::Vector3f(0, 0, 1), safeVertices[0], clippedVertices[0], t);
		sf::Vector2f newT1;
		newT1.x = safeTVertices[0].x + (t * (clippedTVertices[0].x - safeTVertices[0].x));
		newT1.y = safeTVertices[0].y + (t * (clippedTVertices[0].y - safeTVertices[0].y));

		sf::Vector3f two = func::getIntersection(sf::Vector3f(0, 0, c_near), sf::Vector3f(0, 0, 1), safeVertices[0], clippedVertices[1], t);
		sf::Vector2f newT2;
		newT2.x = safeTVertices[0].x + (t * (clippedTVertices[1].x - safeTVertices[0].x));
		newT2.y = safeTVertices[0].y + (t * (clippedTVertices[1].y - safeTVertices[0].y));


		out.push_back(Triangle(one, two, safeVertices[0], newT1, newT2, safeTVertices[0], tri.associatedMtl));
		return 1;
	}

	else if (clippedVertices.size() == 1)
	{
		float t;
		sf::Vector3f one = func::getIntersection(sf::Vector3f(0, 0, c_near), sf::Vector3f(0, 0, 1), safeVertices[0], clippedVertices[0], t);
		sf::Vector2f newT1;
		newT1.x = safeTVertices[0].x + (t * (clippedTVertices[0].x - safeTVertices[0].x));
		newT1.y = safeTVertices[0].y + (t * (clippedTVertices[0].y - safeTVertices[0].y));

		sf::Vector3f two = func::getIntersection(sf::Vector3f(0, 0, c_near), sf::Vector3f(0, 0, 1), safeVertices[1], clippedVertices[0], t);
		sf::Vector2f newT2;
		newT2.x = safeTVertices[1].x + (t * (clippedTVertices[0].x - safeTVertices[1].x));
		newT2.y = safeTVertices[1].y + (t * (clippedTVertices[0].y - safeTVertices[1].y));


		out.push_back(Triangle(one, two, safeVertices[1], newT1, newT2, safeTVertices[1], tri.associatedMtl));
		out.push_back(Triangle(one, safeVertices[0], safeVertices[1], newT1, safeTVertices[0], safeTVertices[1], tri.associatedMtl));

		return 2;
	}
	return 0;


}
