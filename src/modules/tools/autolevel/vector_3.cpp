/*
  vector_3.cpp - Vector library for bed leveling
  Copyright (c) 2012 Lars Brubaker.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <math.h>
#include "vector_3.h"

vector_3::vector_3() : x(0), y(0), z(0) { }

vector_3::vector_3(const float* v) : x(v[0]), y(v[1]), z(v[2]) { }

vector_3::vector_3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) { }

vector_3 vector_3::cross(vector_3 left, vector_3 right)
{
	return vector_3(left.y * right.z - left.z * right.y,
		left.z * right.x - left.x * right.z,
		left.x * right.y - left.y * right.x);
}

vector_3 vector_3::operator=(const vector_3& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	return *this;
}
vector_3 vector_3::operator*(float f)
{
	return vector_3((x * f), (y * f), (z * f));
}
void vector_3::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
}

vector_3 vector_3::operator+(vector_3 v)
{
	return vector_3((x + v.x), (y + v.y), (z + v.z));
}

vector_3 vector_3::operator-(vector_3 v) 
{
	return vector_3((x - v.x), (y - v.y), (z - v.z));
}

float vector_3::operator[](int n)
{
	if (n == 0) return x;
	if (n == 1) return y;
	if (n == 2) return z;
	return -1;	// TODO think what to do in this case
}

vector_3 vector_3::get_normal() 
{
	vector_3 normalized = vector_3(x, y, z);
	normalized.normalize();
	return normalized;
}

float vector_3::get_length() 
{
	float length = sqrt((x * x) + (y * y) + (z * z));
	return length;
}
 
void vector_3::normalize()
{
	float length = get_length();
	x /= length;
	y /= length;
	z /= length;
}

void vector_3::apply_rotation(matrix_3x3 matrix)
{
	float resultX = x * matrix.matrix[3*0+0] + y * matrix.matrix[3*0+1] + z * matrix.matrix[3*0+2];
	float resultY = x * matrix.matrix[3*1+0] + y * matrix.matrix[3*1+1] + z * matrix.matrix[3*1+2];
	float resultZ = x * matrix.matrix[3*2+0] + y * matrix.matrix[3*2+1] + z * matrix.matrix[3*2+2];

	x = resultX;
	y = resultY;
	z = resultZ;
}

void vector_3::debug(const char* title)
{
	THEKERNEL->streams->printf("%s -> x:%f y:%f z:%f\n", title, x, y, z);
}

void apply_rotation_xyz(matrix_3x3 matrix, float &x, float& y, float& z)
{
	vector_3 vector = vector_3(x, y, z);
	vector.apply_rotation(matrix);
	x = vector.x;
	y = vector.y;
	z = vector.z;
}
void apply_rotation_xyz_around_vertex_v(matrix_3x3& matrix, float *v, vector_3& r, vector_3& r_virt)
{
	vector_3 vector = vector_3(v[0], v[1], v[2]) - r_virt;
	vector.apply_rotation(matrix);
	vector = vector + r;
	v[0] = vector.x;
	v[1] = vector.y;
	v[2] = vector.z;
}
void apply_rotation_xyz_around_vertex(matrix_3x3 matrix, float &x, float& y, float& z, float x_r, float y_r, float z_r, float x_r_virt, float y_r_virt, float z_r_virt)
{
	vector_3 r(x_r, y_r, z_r);
	vector_3 r_virt(x_r_virt, y_r_virt, z_r_virt);
	float v[3] = {x, y, z};
	apply_rotation_xyz_around_vertex_v(matrix, v, r, r_virt);
	x = v[0];
	y = v[1];
	z = v[2];
}
void apply_inverted_rotation_xyz_around_vertex_v(matrix_3x3& matrix, float * v, vector_3& r, vector_3& r_virt)
{
	matrix_3x3 inverted_rotation = matrix_3x3::transpose(matrix);
	vector_3 vector = vector_3(v[0], v[1], v[2]) - r;
	vector.apply_rotation(inverted_rotation);
	vector = vector + r_virt;
	v[0] = vector.x;
	v[1] = vector.y;
	v[2] = vector.z;
}
void apply_inverted_rotation_xyz_around_vertex(matrix_3x3 matrix, float &x, float& y, float& z, float x_r, float y_r, float z_r, float x_r_virt, float y_r_virt, float z_r_virt)
{
	vector_3 r(x_r, y_r, z_r);
	vector_3 r_virt(x_r_virt, y_r_virt, z_r_virt);
	float v[3] = {x, y, z};
	apply_inverted_rotation_xyz_around_vertex_v(matrix, v, r, r_virt);
	x = v[0];
	y = v[1];
	z = v[2];
}

matrix_3x3 matrix_3x3::create_from_rows(vector_3 row_0, vector_3 row_1, vector_3 row_2)
{
	//row_0.debug("row_0");
	//row_1.debug("row_1");
	//row_2.debug("row_2");
	matrix_3x3 new_matrix;
	new_matrix.matrix[0] = row_0.x; new_matrix.matrix[1] = row_1.x; new_matrix.matrix[2] = row_2.x;
	new_matrix.matrix[3] = row_0.y; new_matrix.matrix[4] = row_1.y; new_matrix.matrix[5] = row_2.y;
	new_matrix.matrix[6] = row_0.z; new_matrix.matrix[7] = row_1.z; new_matrix.matrix[8] = row_2.z;
	//new_matrix.debug("new_matrix");

	return new_matrix;
}

void matrix_3x3::set_to_identity()
{
	matrix[0] = 1; matrix[1] = 0; matrix[2] = 0;
	matrix[3] = 0; matrix[4] = 1; matrix[5] = 0;
	matrix[6] = 0; matrix[7] = 0; matrix[8] = 1;
}

matrix_3x3 matrix_3x3::create_look_at(vector_3 target)
{
	vector_3 z_row = target.get_normal();
	// we know target.z will be > 0, so, it's safe to divide by it
	vector_3 x_row = vector_3(1, 0, -target.x/target.z).get_normal();
	vector_3 y_row = vector_3(0, 1, -target.y/target.z).get_normal();

	// x_row.debug("x_row");
	// y_row.debug("y_row");
	// z_row.debug("z_row");

 
	// create the matrix already correctly transposed
	matrix_3x3 rot = matrix_3x3::create_from_rows(x_row, y_row, z_row);

	// rot.debug("rot");
	return rot;
}


matrix_3x3 matrix_3x3::transpose(matrix_3x3 original)
{
  matrix_3x3 new_matrix;
  new_matrix.matrix[0] = original.matrix[0]; new_matrix.matrix[1] = original.matrix[3]; new_matrix.matrix[2] = original.matrix[6]; 
  new_matrix.matrix[3] = original.matrix[1]; new_matrix.matrix[4] = original.matrix[4]; new_matrix.matrix[5] = original.matrix[7]; 
  new_matrix.matrix[6] = original.matrix[2]; new_matrix.matrix[7] = original.matrix[5]; new_matrix.matrix[8] = original.matrix[8];
  return new_matrix;
}

void matrix_3x3::debug(const char* title)
{
	THEKERNEL->streams->printf("%s\n", title);

	int count = 0;
	for(int i=0; i<3; i++)
	{
		for(int j=0; j<3; j++)
		{
			THEKERNEL->streams->printf("%f ", matrix[count]);
			count++;
		}

		THEKERNEL->streams->printf("\n");
	}
}

