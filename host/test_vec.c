//Mac-side tests for vec.h and mat.h: compares against float math
//build and run:  make -C host test
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "mat.h"

static int failures = 0;
static const double LSB = 1.0 / 65536; //smallest fx step

static void check(const char *name, double got, double want, double tol)
{
	if (fabs(got - want) > tol) {
		printf("FAIL %-32s got %.6f want %.6f\n", name, got, want);
		failures++;
	}
}

static void check_vec(const char *name, vec3 v, double x, double y, double z, double tol)
{
	check(name, fx_to_float(v.x), x, tol);
	check(name, fx_to_float(v.y), y, tol);
	check(name, fx_to_float(v.z), z, tol);
}

static vec3 V(double x, double y, double z)
{
	return vec3_make((fx)lround(x * 65536), (fx)lround(y * 65536), (fx)lround(z * 65536));
}

static void test_vec_basics(void)
{
	vec3 a = V(1, 2, 3), b = V(-4, 0.5, 2);

	check_vec("add", vec3_add(a, b), -3, 2.5, 5, 0);
	check_vec("sub", vec3_sub(a, b), 5, 1.5, 1, 0);
	check_vec("neg", vec3_neg(a), -1, -2, -3, 0);
	check_vec("scale by 0.5", vec3_scale(a, FX(0.5)), 0.5, 1, 1.5, 0);
	check("dot", fx_to_float(vec3_dot(a, b)), -4 + 1 + 6, LSB);
	check("dot perpendicular = 0", vec3_dot(V(1, 0, 0), V(0, 1, 0)), 0, 0);
}

static void test_cross(void)
{
	check_vec("x cross y = z", vec3_cross(V(1, 0, 0), V(0, 1, 0)), 0, 0, 1, 0);
	check_vec("y cross z = x", vec3_cross(V(0, 1, 0), V(0, 0, 1)), 1, 0, 0, 0);
	check_vec("y cross x = -z", vec3_cross(V(0, 1, 0), V(1, 0, 0)), 0, 0, -1, 0);
	check_vec("parallel = 0", vec3_cross(V(2, 2, 2), V(1, 1, 1)), 0, 0, 0, 0);
	check_vec("general", vec3_cross(V(1, 2, 3), V(4, 5, 6)), -3, 6, -3, LSB);
}

static void test_length_normalize(void)
{
	check("length (3,4,0)", fx_to_float(vec3_length(V(3, 4, 0))), 5, LSB);
	check("length (300,400,0) no overflow", fx_to_float(vec3_length(V(300, 400, 0))), 500, LSB);
	check("length (0.1,0.2,0.2)", fx_to_float(vec3_length(V(0.1, 0.2, 0.2))), 0.3, 2 * LSB);

	check_vec("normalize (3,4,0)", vec3_normalize(V(3, 4, 0)), 0.6, 0.8, 0, 2 * LSB);
	check("normalized length = 1", fx_to_float(vec3_length(vec3_normalize(V(-7, 2, 11)))), 1, 3 * LSB);
	check_vec("normalize zero stays zero", vec3_normalize(V(0, 0, 0)), 0, 0, 0, 0);
}

static void test_rotations(void)
{
	mat3 m;

	//90 degree turns of the unit axes (right-hand rule)
	mat3_rotate_z(&m, FX_DEG(90));
	check_vec("Rz(90) x -> y", mat3_mul_vec(&m, V(1, 0, 0)), 0, 1, 0, LSB);
	mat3_rotate_x(&m, FX_DEG(90));
	check_vec("Rx(90) y -> z", mat3_mul_vec(&m, V(0, 1, 0)), 0, 0, 1, LSB);
	mat3_rotate_y(&m, FX_DEG(90));
	check_vec("Ry(90) z -> x", mat3_mul_vec(&m, V(0, 0, 1)), 1, 0, 0, LSB);

	//compare against float rotation at many angles
	double worst = 0;
	for (fx_angle a = 0; a < FX_ANGLE_FULL; a += 7) {
		double r = a * 2 * M_PI / FX_ANGLE_FULL;
		mat3_rotate_z(&m, a);
		vec3 v = mat3_mul_vec(&m, V(0.6, -0.8, 0.5));
		double ex = 0.6 * cos(r) + 0.8 * sin(r), ey = 0.6 * sin(r) - 0.8 * cos(r);
		worst = fmax(worst, fabs(fx_to_float(v.x) - ex));
		worst = fmax(worst, fabs(fx_to_float(v.y) - ey));
		check("Rz keeps z", fx_to_float(v.z), 0.5, 0);
	}
	check("Rz vs float worst error", worst, 0, 4 * LSB);
	printf("rotation worst error vs float: %.2e\n", worst);
}

static void test_mat_ops(void)
{
	mat3 id, a, b, ab, t, back;
	vec3 v = V(0.3, -0.7, 0.2);

	mat3_identity(&id);
	check_vec("identity * v = v", mat3_mul_vec(&id, v), 0.3, -0.7, 0.2, LSB);

	//(A*B)*v must equal A*(B*v)
	mat3_rotate_x(&a, FX_DEG(30));
	mat3_rotate_y(&b, FX_DEG(75));
	mat3_mul(&ab, &a, &b);
	vec3 one_step = mat3_mul_vec(&ab, v);
	vec3 two_steps = mat3_mul_vec(&a, mat3_mul_vec(&b, v));
	check_vec("(A*B)v = A(Bv)", one_step, fx_to_float(two_steps.x), fx_to_float(two_steps.y),
		fx_to_float(two_steps.z), 3 * LSB);

	//rotating then rotating back by the transpose returns the original
	mat3_rotate_xyz(&a, FX_DEG(20), FX_DEG(-50), FX_DEG(110));
	mat3_transpose(&t, &a);
	vec3 there_and_back = mat3_mul_vec(&t, mat3_mul_vec(&a, v));
	check_vec("transpose undoes rotation", there_and_back, 0.3, -0.7, 0.2, 4 * LSB);

	//a rotation doesn't change length
	check("rotation keeps length", fx_to_float(vec3_length(mat3_mul_vec(&a, v))),
		fx_to_float(vec3_length(v)), 4 * LSB);

	//mat3_mul writing into one of its inputs
	mat3_rotate_z(&back, FX_DEG(40));
	mat3_mul(&back, &back, &back); //40 + 40 = 80 degrees
	mat3_rotate_z(&b, FX_DEG(80));
	check("aliased mul (m = m*m)", fx_to_float(back.m[0][1]), fx_to_float(b.m[0][1]), 3 * LSB);
}

int main(void)
{
	test_vec_basics();
	test_cross();
	test_length_normalize();
	test_rotations();
	test_mat_ops();

	if (failures == 0) printf("all vec/mat tests passed\n");
	else printf("%d failure(s)\n", failures);
	return failures != 0;
}
