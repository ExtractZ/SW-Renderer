//Mac-side tests for fixed.h: compares every function against float math
//build and run:  make -C host test
#include <math.h>
#include <stdio.h>
#include "fixed.h"

static int failures = 0;

static void check(const char *name, double got, double want, double tol)
{
	if (fabs(got - want) > tol) {
		printf("FAIL %-28s got %.6f want %.6f\n", name, got, want);
		failures++;
	}
}

static void test_constants(void)
{
	check("FX(1.0)", FX(1.0), 65536, 0);
	check("FX(0.5)", FX(0.5), 32768, 0);
	check("FX(-2.25)", FX(-2.25), -147456, 0);
	check("FX(0.1) rounds", FX(0.1), 6554, 0); //0.1*65536 = 6553.6
}

static void test_conversions(void)
{
	check("from_int(-7)", fx_to_float(fx_from_int(-7)), -7, 0);
	check("to_int(2.75)", fx_to_int(FX(2.75)), 2, 0);
	check("to_int(-2.25) floors", fx_to_int(FX(-2.25)), -3, 0);
	check("round(2.5)", fx_round(FX(2.5)), 3, 0);
	check("round(2.49)", fx_round(FX(2.49)), 2, 0);
	check("frac(-2.25)", fx_to_float(fx_frac(FX(-2.25))), 0.75, 0);
}

static void test_mul_div(void)
{
	const double step = 1.0 / 65536;
	const double vals[] = { 0, 1, -1, 0.5, -0.5, 3.14159, -2.71828, 100.25, -181.0, 0.001 };
	const int n = sizeof(vals) / sizeof(vals[0]);

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			double a = vals[i], b = vals[j];
			fx fa = (fx)lround(a * 65536), fb = (fx)lround(b * 65536);
			double ra = fa / 65536.0, rb = fb / 65536.0; //what the fx values really hold

			check("mul", fx_to_float(fx_mul(fa, fb)), ra * rb, 2 * step);
			if (fb != 0 && fabs(ra / rb) < 32000)
				check("div", fx_to_float(fx_div(fa, fb)), ra / rb, 2 * step + fabs(ra / rb) * 1e-6);
		}
	}
	check("mul big (no overflow)", fx_to_float(fx_mul(FX(150.0), FX(200.0))), 30000, 0);
	check("div by zero saturates", fx_div(FX(1.0), 0), FX_MAX, 0);
	check("div -x by zero", fx_div(FX(-1.0), 0), FX_MIN, 0);
}

static void test_sqrt(void)
{
	const double vals[] = { 0, 1, 2, 4, 0.25, 0.01, 10, 100, 12345.678, 32767 };
	for (unsigned i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
		fx a = (fx)lround(vals[i] * 65536);
		check("sqrt", fx_to_float(fx_sqrt(a)), sqrt(a / 65536.0), 2.0 / 65536);
	}
	check("sqrt(-1) = 0", fx_sqrt(FX(-1.0)), 0, 0);
}

static void test_trig(void)
{
	double worst = 0;
	for (int a = -FX_ANGLE_FULL; a <= 2 * FX_ANGLE_FULL; a++) {
		double rad = a * 2 * M_PI / FX_ANGLE_FULL;
		double es = fabs(fx_to_float(fx_sin(a)) - sin(rad));
		double ec = fabs(fx_to_float(fx_cos(a)) - cos(rad));
		if (es > worst) worst = es;
		if (ec > worst) worst = ec;
	}
	check("sin/cos worst error", worst, 0, 1.0 / 65536);
	check("sin(90)", fx_sin(FX_DEG(90)), FX_ONE, 0);
	check("cos(180)", fx_cos(FX_DEG(180)), -FX_ONE, 0);
	check("sin(-90)", fx_sin(FX_DEG(-90)), -FX_ONE, 0);
	printf("sin/cos worst error over 3 turns: %.2e\n", worst);
}

int main(void)
{
	test_constants();
	test_conversions();
	test_mul_div();
	test_sqrt();
	test_trig();

	if (failures == 0) printf("all fixed-point tests passed\n");
	else printf("%d failure(s)\n", failures);
	return failures != 0;
}
