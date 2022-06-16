#pragma once
#include "Rasterizer.hpp"
#include <iostream>



Rasterizer::Rasterizer(int _w_width, int _w_height, float _c_near, float _c_far, int _fov) :
	w_width(_w_width), w_height(_w_height), c_near(_c_near), c_far(_c_far), fov(_fov)
{
	const float pi = 3.141592653;
	aspect_ratio = w_width / w_height;

	//canvas size and dimensions
	nWidth = tan((fov / 2) * pi / 180) * c_near * 2;
	nHeight = nWidth;
	fWidth = tan((fov / 2) * pi / 180) * c_far * 2;
	fHeight = fWidth;

	c_top = nHeight / 2;
	c_right = nWidth / 2;
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
	//p_mat[0][0] = aspect_ratio * 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	//p_mat[1][1] = 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	//p_mat[2][2] = -c_far / (c_far - c_near);
	//p_mat[2][3] = 1;
	//p_mat[3][2] = -c_far * c_near / (c_far - c_near);
	//p_mat[3][3] = 0;

	p_mat[0][0] = (2 * c_near) / (c_right - c_left);
	p_mat[1][1] = (2 * c_near) / (c_top - c_bot);
	p_mat[2][0] = (c_right + c_left) / (c_right - c_left);
	p_mat[2][1] = (c_top + c_bot) / (c_top - c_bot);
	p_mat[2][2] = -(c_far + c_near) / (c_far - c_near);
	p_mat[2][3] = 1;
	p_mat[3][2] = -(2 * c_far * c_near) / (c_far - c_near);
	p_mat[3][3] = 0;

}

bool Rasterizer::project_triangle(Triangle& tri, float mat[4][4])
{
	func::vecXmatrix(tri.transVerts[0], mat, tri.projVerts[0], true);
	func::vecXmatrix(tri.transVerts[1], mat, tri.projVerts[1], true);
	func::vecXmatrix(tri.transVerts[2], mat, tri.projVerts[2], true);


	//if (tri.projVerts[0].x > tri.projVerts[0].w && tri.projVerts[1].x > tri.projVerts[1].w && tri.projVerts[2].x > tri.projVerts[2].w)
	//	return false;
	//if (tri.projVerts[0].x < -tri.projVerts[0].w && tri.projVerts[1].x < -tri.projVerts[1].w && tri.projVerts[2].x < -tri.projVerts[2].w)
	//	return false;
	//if (tri.projVerts[0].y > tri.projVerts[0].w && tri.projVerts[1].y > tri.projVerts[1].w && tri.projVerts[2].y > tri.projVerts[2].w)
	//	return false;
	//if (tri.projVerts[0].y < -tri.projVerts[0].w && tri.projVerts[1].y < -tri.projVerts[1].w && tri.projVerts[2].y < -tri.projVerts[2].w)
	//	return false;



	for (int i = 0; i < 3; ++i)
	{
		tri.projVerts[i].x = floor((tri.projVerts[i].x + 1) / 2 * w_width);
		tri.projVerts[i].y = floor((1 - tri.projVerts[i].y) / 2 * w_height);
	}
	tri.bLeft = std::min({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x });
	tri.bTop = std::min({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y });
	tri.bRight = std::max({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x });
	tri.bBot = std::max({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y });
	//out.push_back(&tri);
	return true;
}


int Rasterizer::clip_triangle_near(Triangle& tri, std::vector<Triangle*>& outputTris, std::vector<Triangle*>& trisToDelete) const
{
	if (tri.transVerts[0].z >= c_near && tri.transVerts[1].z >= c_near && tri.transVerts[2].z >= c_near)
	{
		outputTris.push_back(&tri);
		return 1;
	}
	else if (tri.transVerts[0].z < c_near && tri.transVerts[1].z < c_near && tri.transVerts[2].z < c_near)
	{
		return 0;
	}

	std::vector<vec4> clippedVertices;
	std::vector<vec4> safeVertices;
	std::vector<vec2> clippedTVertices;
	std::vector<vec2> safeTVertices;

	for (int i = 0; i < 3; ++i)
	{
		if (tri.transVerts[i].z < c_near)
		{
			clippedVertices.push_back(tri.transVerts[i]);
			clippedTVertices.push_back(tri.tCoords[i]);
		}
		else
		{
			safeVertices.push_back(tri.transVerts[i]);
			safeTVertices.push_back(tri.tCoords[i]);
		}
	}

	// only need to create one new triangle
	if (clippedVertices.size() == 2)
	{
		float t;
		vec4 one = func::getIntersection(vec4(0, 0, c_near), vec4(0, 0, 1), safeVertices[0], clippedVertices[0], t);
		vec2 newT1;
		newT1.x = safeTVertices[0].x + (t * (clippedTVertices[0].x - safeTVertices[0].x));
		newT1.y = safeTVertices[0].y + (t * (clippedTVertices[0].y - safeTVertices[0].y));

		vec4 two = func::getIntersection(vec4(0, 0, c_near), vec4(0, 0, 1), safeVertices[0], clippedVertices[1], t);
		vec2 newT2;
		newT2.x = safeTVertices[0].x + (t * (clippedTVertices[1].x - safeTVertices[0].x));
		newT2.y = safeTVertices[0].y + (t * (clippedTVertices[1].y - safeTVertices[0].y));

		Triangle *clippedTri = new Triangle(tri.verts[0], tri.verts[1], tri.verts[2], newT1, newT2, safeTVertices[0], tri.associatedMtl);
		clippedTri->transVerts[0] = one;
		clippedTri->transVerts[1] = two;
		clippedTri->transVerts[2] = safeVertices[0];
		outputTris.push_back(clippedTri);
		trisToDelete.push_back(clippedTri);
		return 1;
	}

	else if (clippedVertices.size() == 1)
	{
		float t;
		vec4 one = func::getIntersection(vec4(0, 0, c_near), vec4(0, 0, 1), safeVertices[0], clippedVertices[0], t);
		vec2 newT1;
		newT1.x = safeTVertices[0].x + (t * (clippedTVertices[0].x - safeTVertices[0].x));
		newT1.y = safeTVertices[0].y + (t * (clippedTVertices[0].y - safeTVertices[0].y));

		vec4 two = func::getIntersection(vec4(0, 0, c_near), vec4(0, 0, 1), safeVertices[1], clippedVertices[0], t);
		vec2 newT2;
		newT2.x = safeTVertices[1].x + (t * (clippedTVertices[0].x - safeTVertices[1].x));
		newT2.y = safeTVertices[1].y + (t * (clippedTVertices[0].y - safeTVertices[1].y));

		Triangle *clippedTri = new Triangle(tri.verts[0], tri.verts[1], tri.verts[2], newT1, newT2, safeTVertices[1], tri.associatedMtl);
		clippedTri->transVerts[0] = one;
		clippedTri->transVerts[1] = two;
		clippedTri->transVerts[2] = safeVertices[1];
		Triangle *clippedTri2 = new Triangle(tri.verts[0], tri.verts[1], tri.verts[2], newT1, safeTVertices[0], safeTVertices[1], tri.associatedMtl);
		clippedTri2->transVerts[0] = one;
		clippedTri2->transVerts[1] = safeVertices[0];
		clippedTri2->transVerts[2] = safeVertices[1];
		outputTris.push_back(clippedTri);
		outputTris.push_back(clippedTri2);
		trisToDelete.push_back(clippedTri);
		trisToDelete.push_back(clippedTri2);

		return 2;
	}
	return 0;


}

