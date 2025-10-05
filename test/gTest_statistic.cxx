#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <valarray>
#include <cstdint>
#include <random>

#include <Minuit2/MnSimplex.h>

#include "DSTfits/FITSstatistic.h"

using DSL::stat::Percentil;

#pragma region valarray ctor

TEST(PercentilTest, ArrayDoubleEvalAndOperator)
{
	std::valarray<double> v{-2.0, -4.0, -1.0, -3.0};
	Percentil p(v);

	// fraction of values <= 2.5 is 2/4 = 0.5
	EXPECT_DOUBLE_EQ(p.Eval(-2.5) , 0.5);
	EXPECT_DOUBLE_EQ(p.Eval(-3.25), 1./4.);
	EXPECT_DOUBLE_EQ(p.Eval(-1.75), 3./4.);
	EXPECT_DOUBLE_EQ(p.Eval(-1), 1.0);
	EXPECT_DOUBLE_EQ(p.Eval(-1), 1.0);
	EXPECT_DOUBLE_EQ(p.Eval(-4), 0.0);
	EXPECT_DOUBLE_EQ(p.Eval(-5), 0.0);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -2.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {-1}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {-1}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -1.75, 1e-4);
}

TEST(PercentilTest, ArrayFloatEvalAndOperator)
{
	std::valarray<float> v{-2.0, -4.0, -1.0, -3.0};
	Percentil p(v);

	// fraction of values <= 2.5 is 2/4 = 0.5
	EXPECT_DOUBLE_EQ(p.Eval(-2.5), 0.5);
	EXPECT_DOUBLE_EQ(p.Eval(-3.25)  , 1./4.);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -2.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {-1}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {-1}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -1.75, 1e-4);
}

TEST(PercentilTest, ArrayInt64EvalAndOperator)
{
	std::valarray<int64_t> v{1, 4, -3, -1, 0, -5, -2, 3, -4, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(-0.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -0.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -2.75, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 2.5, 1e-4);
}

TEST(PercentilTest, ArrayInt32EvalAndOperator)
{
	std::valarray<int32_t> v{1, 4, -3, -1, 0, -5, -2, 3, -4, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(-0.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -0.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -2.75, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 2.5, 1e-4);
}

TEST(PercentilTest, ArrayInt16EvalAndOperator)
{
	std::valarray<int16_t> v{1, 4, -3, -1, 0, -5, -2, 3, -4, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(-0.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -0.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -2.75, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 2.5, 1e-4);
}

TEST(PercentilTest, ArrayUInt64EvalAndOperator)
{
	std::valarray<uint64_t> v{1, 4, 6, 9, 0, 8, 7, 3, 10, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(5.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {5}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 5.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 7.75, 1e-4);
}

TEST(PercentilTest, ArrayUInt32EvalAndOperator)
{
	std::valarray<uint32_t> v{1, 4, 6, 9, 0, 8, 7, 3, 10, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(5.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {5}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 5.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 7.75, 1e-4);
}

TEST(PercentilTest, ArrayUInt16EvalAndOperator)
{
	std::valarray<uint32_t> v{1, 4, 6, 9, 0, 8, 7, 3, 10, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(5.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {5}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 5.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 7.75, 1e-4);
}

#pragma endregion
#pragma region vector ctor

TEST(PercentilTest, VectorDoubleEvalAndOperator)
{
	std::vector<double> v{-2.0, -4.0, -1.0, -3.0};
	Percentil p(v);

	// fraction of values <= 2.5 is 2/4 = 0.5
	EXPECT_DOUBLE_EQ(p.Eval(-2.5), 0.5);
	EXPECT_DOUBLE_EQ(p.Eval(-3.25), 1./4.);
	EXPECT_DOUBLE_EQ(p.Eval(-1.75), 3./4.);
	EXPECT_DOUBLE_EQ(p.Eval(-1), 1.0);
	EXPECT_DOUBLE_EQ(p.Eval(-1), 1.0);
	EXPECT_DOUBLE_EQ(p.Eval(-4), 0.0);
	EXPECT_DOUBLE_EQ(p.Eval(-5), 0.0);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -2.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {-1}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {-1}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -1.75, 1e-4);
}

TEST(PercentilTest, VectorFloatEvalAndOperator)
{
	std::vector<float> v{-2.0, -4.0, -1.0, -3.0};
	Percentil p(v);

	// fraction of values <= 2.5 is 2/4 = 0.5
	EXPECT_DOUBLE_EQ(p.Eval(-2.5), 0.5);
	EXPECT_DOUBLE_EQ(p.Eval(-3.25)  , 1./4.);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -2.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {-1}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {-1}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(min.UserState().Params()[0], -1.75, 1e-4);
}

TEST(PercentilTest, VectorInt64EvalAndOperator)
{
	std::vector<int64_t> v{1, 4, -3, -1, 0, -5, -2, 3, -4, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(-0.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -0.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -2.75, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 2.5, 1e-4);
}

TEST(PercentilTest, VectorInt32EvalAndOperator)
{
	std::vector<int32_t> v{1, 4, -3, -1, 0, -5, -2, 3, -4, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(-0.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -0.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -2.75, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 2.5, 1e-4);
}

TEST(PercentilTest, VectorInt16EvalAndOperator)
{
	std::vector<int16_t> v{1, 4, -3, -1, 0, -5, -2, 3, -4, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(-0.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {-1}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -0.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], -2.75, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {0}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 2.5, 1e-4);
}

TEST(PercentilTest, VectorUInt64EvalAndOperator)
{
	std::vector<uint64_t> v{1, 4, 6, 9, 0, 8, 7, 3, 10, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(5.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {5}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 5.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 7.75, 1e-4);
}

TEST(PercentilTest, VectorUInt32EvalAndOperator)
{
	std::vector<uint32_t> v{1, 4, 6, 9, 0, 8, 7, 3, 10, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(5.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {5}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 5.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 7.75, 1e-4);
}

TEST(PercentilTest, VectorUInt16EvalAndOperator)
{
	std::vector<uint32_t> v{1, 4, 6, 9, 0, 8, 7, 3, 10, 5};
	Percentil p(v);

	// fraction of values <= 0
	EXPECT_DOUBLE_EQ(p.Eval(5.5), 0.5);

	ROOT::Minuit2::VariableMetricMinimizer Minimizer = ROOT::Minuit2::VariableMetricMinimizer();
	ROOT::Minuit2::FunctionMinimum min = Minimizer.Minimize(p, {5}, {1e-4},2);

	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.5, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 5.5, 1e-4);

	p.SetPercentil(0.25);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.25, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 3.25, 1e-4);

	p.SetPercentil(0.75);
	min = Minimizer.Minimize(p, {5}, {1e-4},2);
	EXPECT_TRUE(min.IsValid());
	EXPECT_NEAR(p.Eval(min.UserState().Params()[0]), 0.75, 1e-4);
	EXPECT_NEAR(min.UserState().Params()[0], 7.75, 1e-4);
}

#pragma endregion

#pragma region test copy and move

TEST(PercentilTest, CopyCtor)
{
	std::valarray<double> v{-2.0, -4.0, -1.0, -3.0};
	Percentil p1(v);
	Percentil p2 = p1;

	EXPECT_DOUBLE_EQ(p2.Eval(-2.5), 0.5);
	EXPECT_DOUBLE_EQ(p2.Eval(-3.25), 1./4.);
	EXPECT_DOUBLE_EQ(p2.Eval(-1.75), 3./4.);

	Percentil p3(p2);
	EXPECT_DOUBLE_EQ(p2.Eval(-2.5),  p3.Eval(-2.5));
	EXPECT_DOUBLE_EQ(p2.Eval(-3.25), p3.Eval(-3.25));
	EXPECT_DOUBLE_EQ(p2.Eval(-1.75), p3.Eval(-1.75));
}

#pragma endregion

#pragma region test parrallelism

TEST(PercentilTest, DoubleParallelEval)
{
    const size_t N = 1000000;
    std::default_random_engine re;
    re.seed(123456); // deterministic for test
    std::normal_distribution<double> norm(0.0, 1.0);   // normal( mean=0, sigma=1 )

    std::vector<double> events;
    events.reserve(N);
    for (size_t i = 0; i < N; ++i) events.push_back(norm(re));

    // build Percentil from the generated samples
    Percentil p(events);

    // basic statistical check: CDF at 0 should be ~0.5 (tolerance ~2% for N=10000)
    double cdf0 = p.Eval(0.0);
    EXPECT_NEAR(cdf0, 0.5, 0.02);

    // exercise Eval() concurrently
    std::vector<std::future<double>> futs;
    for (int t = -4; t <= 3; ++t)
    {
        double thr = static_cast<double>(t);
        futs.emplace_back(std::async(std::launch::async, [&p, thr]() { return p.Eval(thr); }));
    }
    for (auto &f : futs) (void)f.get();

    // robust minimization: simplex -> variable-metric refine
	ROOT::Minuit2::MnMachinePrecision().SetPrecision(1e-14);
	ROOT::Minuit2::MnStrategy str(1);
    str.SetGradientStepTolerance(1e-8);

	ROOT::Minuit2::MnUserParameterState start(std::vector<double>{0.0}, std::vector<double>{1.0});
	ROOT::Minuit2::MnSimplex simplex(p, start, str);
	auto approx = simplex();

    ROOT::Minuit2::VariableMetricMinimizer vm;
    auto refined = vm.Minimize(p,
                               approx.UserState().Params(),
                               approx.UserState().Errors(),
                               1000000);

    ASSERT_TRUE(refined.IsValid());
    double best = refined.UserState().Params()[0];
    EXPECT_NEAR(best, 0.0, 0.03);
    EXPECT_NEAR(p.Eval(best), 0.5, 5e-3);

    // 16% quantile (~-0.994458)
    p.SetPercentil(0.16);
    auto min16 = vm.Minimize(p, {0.0}, {1.0}, 1000);
    ASSERT_TRUE(min16.IsValid());
    EXPECT_NEAR(min16.UserState().Params()[0], -0.994458, 0.005);
    EXPECT_NEAR(p.Eval(min16.UserState().Params()[0]), 0.16, 5e-3);

    // 84% quantile (~0.994458)
    p.SetPercentil(0.84);
    auto min84 = vm.Minimize(p, {0.0}, {1.0}, 1000);
    ASSERT_TRUE(min84.IsValid());
#ifdef Darwinx86_64
    EXPECT_NEAR(min84.UserState().Params()[0],  0.994458, 0.005);
    EXPECT_NEAR(p.Eval(min84.UserState().Params()[0]), 0.84, 5e-3);
#else
    EXPECT_NEAR(min84.UserState().Params()[0],  0.994458, 0.05);
    EXPECT_NEAR(p.Eval(min84.UserState().Params()[0]), 0.84, 5e-2);
#endif
}

TEST(PercentilTest, FloatParallelEval)
{
    const size_t N = 1000000;
    std::default_random_engine re;
    re.seed(123456); // deterministic for test
    std::normal_distribution<float> norm(0.0, 1.0);   // normal( mean=0, sigma=1 )

    std::vector<float> events;
    events.reserve(N);
    for (size_t i = 0; i < N; ++i) events.push_back(norm(re));

    // build Percentil from the generated samples
    Percentil p(events);

    // basic statistical check: CDF at 0 should be ~0.5 (tolerance ~2% for N=10000)
    double cdf0 = p.Eval(0.0);
    EXPECT_NEAR(cdf0, 0.5, 0.02);

    // exercise Eval() concurrently
    std::vector<std::future<double>> futs;
    for (int t = -4; t <= 3; ++t)
    {
        double thr = static_cast<double>(t);
        futs.emplace_back(std::async(std::launch::async, [&p, thr]() { return p.Eval(thr); }));
    }
    for (auto &f : futs) (void)f.get();

    // robust minimization: simplex -> variable-metric refine
	ROOT::Minuit2::MnMachinePrecision().SetPrecision(1e-14);
	ROOT::Minuit2::MnStrategy str(1);
    str.SetGradientStepTolerance(1e-8);

	ROOT::Minuit2::MnUserParameterState start(std::vector<double>{0.0}, std::vector<double>{1.0});
	ROOT::Minuit2::MnSimplex simplex(p, start, str);
	auto approx = simplex();

    ROOT::Minuit2::VariableMetricMinimizer vm;
    auto refined = vm.Minimize(p,
                               approx.UserState().Params(),
                               approx.UserState().Errors(),
                               1000000);

    ASSERT_TRUE(refined.IsValid());
    double best = refined.UserState().Params()[0];
    EXPECT_NEAR(best, 0.0, 0.1);
    EXPECT_NEAR(p.Eval(best), 0.5, 0.05);

    // 16% quantile (~-0.994458)
    p.SetPercentil(0.16);
    auto min16 = vm.Minimize(p, {0.0}, {1.0}, 1000);
    ASSERT_TRUE(min16.IsValid());
    EXPECT_NEAR(min16.UserState().Params()[0], -0.994458, 0.1);
    EXPECT_NEAR(p.Eval(min16.UserState().Params()[0]), 0.16, 0.05);

    // 84% quantile (~0.994458)
    p.SetPercentil(0.84);
    auto min84 = vm.Minimize(p, {0.0}, {1.0}, 1000);
    ASSERT_TRUE(min84.IsValid());
    EXPECT_NEAR(min84.UserState().Params()[0],  0.994458, 0.1);
    EXPECT_NEAR(p.Eval(min84.UserState().Params()[0]), 0.84, 0.05);
}

#pragma endregion