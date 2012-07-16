#include "math.hpp"
#include <stdio.h>
#include <cmath>

vec3
sub(vec3 *a, vec3 *b) {
	vec3 result;
	result.x = a->x - b->x;
	result.y = a->y - b->y;
	result.z = a->z - b->z;
	return result;
}

vec3
mul(vec3 *a, double b) {
	vec3 result;
	result.x = a->x * b;
	result.y = a->y * b;
	result.z = a->z * b;
	return result;
}

vec3
div(vec3 *a, double b) {
	vec3 result;
	result.x = a->x / b;
	result.y = a->y / b;
	result.z = a->z / b;
	return result;
}

vec3
add(vec3 *a, vec3 *b) {
	vec3 result;
	result.x = a->x + b->x;
	result.y = a->y + b->y;
	result.z = a->z + b->z;
	return result;
}

vec3
rot_x (vec3 *t, vec3 *r)
{
	vec3 result;
	result.x = t->x;
	result.y = cos(r->x) * t->y - sin(r->x) * t->z;
	result.z = sin(r->x) * t->y + cos(r->x) * t->z;
	return result;
}

vec3
rot_y (vec3 *t, vec3 *r)
{
	vec3 result;
	result.x = cos(r->y) * t->x + sin(r->y) * t->z;
	result.y = t->y;
	result.z = -sin(r->y) * t->x + cos(r->y) * t->z;
	return result;
}

vec3
rot_z (vec3 *t, vec3 *r)
{
	vec3 result;
	result.x = cos(r->z) * t->x - sin(r->z) * t->y;
	result.y = sin(r->z) * t->x + cos(r->z) * t->y;
	result.z = t->z;
	return result;
}

// NOTE: from XYZ euler!
mat3x3
make_rotation_matrix(double x, double y, double z)
{
	mat3x3 result;

	const double c1 = cos(x);
	const double c2 = cos(y);
	const double c3 = cos(z);

	const double s1 = sin(x);
	const double s2 = sin(y);
	const double s3 = sin(z);

	result.a11 = c2 * c3;
	result.a12 = c2 * s3;
	result.a13 = -s2;

	result.a21 = c3 * s1 * s2 - c1 * s3;
	result.a22 = c1 * c3 + s1 * s2 * s3;
	result.a23 = c2 * s1;

	result.a31 = s1 * s3 + c1 * c3 * s2;
	result.a32 = c1 * s2 * s3 - c3 * s1;
	result.a33 = c1 * c2;

	return result;
}

mat3x3
make_rotation_matrix(vec3 *v)
{
	return make_rotation_matrix(v->x, v->y, v->z);
}

mat3x3
matrix_transpose(mat3x3 *mat)
{
	mat3x3 result;

	result.a11 = mat->a11;
	result.a12 = mat->a21;
	result.a13 = mat->a31;

	result.a21 = mat->a12;
	result.a22 = mat->a22;
	result.a23 = mat->a32;

	result.a31 = mat->a13;
	result.a32 = mat->a23;
	result.a33 = mat->a33;

	return result;
}

vec3
mul(mat3x3 *mat, vec3 *vec)
{
	vec3 result;

	result.x = mat->a11 * vec->x + mat->a12 * vec->y + mat->a13 * vec->z;
	result.y = mat->a21 * vec->x + mat->a22 * vec->y + mat->a23 * vec->z;
	result.z = mat->a31 * vec->x + mat->a32 * vec->y + mat->a33 * vec->z;

	return result;
}

mat3x3
mul(mat3x3 *a, mat3x3 *b)
{
	mat3x3 result;

	result.a11 = a->a11 * b->a11 + a->a12 * b->a21 + a->a13 * b->a31;
	result.a12 = a->a11 * b->a12 + a->a12 * b->a22 + a->a13 * b->a32;
	result.a13 = a->a11 * b->a13 + a->a12 * b->a23 + a->a13 * b->a33;

	result.a21 = a->a21 * b->a11 + a->a22 * b->a21 + a->a23 * b->a31;
	result.a22 = a->a21 * b->a12 + a->a22 * b->a22 + a->a23 * b->a32;
	result.a23 = a->a21 * b->a13 + a->a22 * b->a23 + a->a23 * b->a33;

	result.a31 = a->a31 * b->a11 + a->a32 * b->a21 + a->a33 * b->a31;
	result.a32 = a->a31 * b->a12 + a->a32 * b->a22 + a->a33 * b->a32;
	result.a33 = a->a31 * b->a13 + a->a32 * b->a23 + a->a33 * b->a33;

	return result;
}

mat3x3
extract_rot(mat4x4 *m)
{
	mat3x3 result;

	result.a11 = m->a11;
	result.a12 = m->a12;
	result.a13 = m->a13;

	result.a21 = m->a21;
	result.a22 = m->a22;
	result.a23 = m->a23;

	result.a31 = m->a31;
	result.a32 = m->a32;
	result.a33 = m->a33;

	return result;
}

vec3
extract_trans(mat4x4 *m)
{
	vec3 result;
	result.x = m->a14;
	result.y = m->a24;
	result.z = m->a34;
	return result;
}

void
print_vec3(vec3 *v) {
	printf("V = [%+4.4f %+4.4f %+4.4f]\n", v->x, v->y, v->z);
}

void
print_mat3x3(mat3x3 *m) {
	printf("M = [%+4.4f %+4.4f %+4.4f\n",  m->a11, m->a12, m->a13);
	printf("     %+4.4f %+4.4f %+4.4f\n",  m->a21, m->a22, m->a23);
	printf("     %+4.4f %+4.4f %+4.4f]\n", m->a31, m->a32, m->a33);
}

void
print_mat4x4(mat4x4 *m) {
	printf("M = [%+4.4f %+4.4f %+4.4f %+4.4f\n",  m->a11, m->a12, m->a13, m->a14);
	printf("     %+4.4f %+4.4f %+4.4f %+4.4f\n",  m->a21, m->a22, m->a23, m->a24);
	printf("     %+4.4f %+4.4f %+4.4f %+4.4f\n",  m->a31, m->a32, m->a33, m->a34);
	printf("     %+4.4f %+4.4f %+4.4f %+4.4f]\n", m->a41, m->a42, m->a43, m->a44);

}

vec3
mat3x3_to_euler(mat3x3 *m)
{
	vec3 result;

	result.x = atan2(m->a32, m->a33);
	result.y = atan2(-m->a31, sqrt(m->a32 * m->a32 + m->a33 * m->a33));
	result.z = atan2(m->a21, m->a11);

	return result;
}
