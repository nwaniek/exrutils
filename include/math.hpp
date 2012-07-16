#ifndef __MATH_HPP__3C378DD8_66DF_4E99_BB57_E65058FBB871
#define __MATH_HPP__3C378DD8_66DF_4E99_BB57_E65058FBB871

struct vec3 {
	double x, y, z;
};

struct mat3x3 {
	double a11, a12, a13;
	double a21, a22, a23;
	double a31, a32, a33;
};

struct mat4x4 {
	double a11, a12, a13, a14;
	double a21, a22, a23, a24;
	double a31, a32, a33, a34;
	double a41, a42, a43, a44;
};

vec3 sub(vec3 *a, vec3 *b);
vec3 mul(vec3 *a, double b);
vec3 div(vec3 *a, double b);
vec3 add(vec3 *a, vec3 *b);
vec3 rot_x (vec3 *t, vec3 *r);
vec3 rot_y (vec3 *t, vec3 *r);
vec3 rot_z (vec3 *t, vec3 *r);

mat3x3 make_rotation_matrix(double x, double y, double z);
mat3x3 make_rotation_matrix(vec3 *v);
mat3x3 matrix_transpose(mat3x3 *mat);

vec3 mul(mat3x3 *mat, vec3 *vec);
mat3x3 mul(mat3x3 *a, mat3x3 *b);
mat3x3 extract_rot(mat4x4 *m);
vec3 extract_trans(mat4x4 *m);
vec3 mat3x3_to_euler(mat3x3 *m);

void print_vec3(vec3 *v);
void print_mat3x3(mat3x3 *m);
void print_mat4x4(mat4x4 *m);


#endif /* __MATH_HPP__3C378DD8_66DF_4E99_BB57_E65058FBB871 */

