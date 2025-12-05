#include <gtest/gtest.h>
#include <DSTfits/FITStable.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <tuple>
#include <random>
#include <thread>


using namespace DSL;

namespace
{

    FITStable CreateSampleTable()
    {
        FITStable table;

        auto colInt = std::make_shared<FITScolumn<int32_t>>("COL_INT", tint, "", 1);
        //                                        0   1    2   3   4   5   6   7    8    9    10   11
        for (int32_t value : std::vector<int32_t>{-5, -1,  0,  4,  9, 12,  3,  3,  28,  -5,  -20,  10})
        {
            colInt->push_back(value);
        }
        table.InsertColumn(colInt);

        auto colDouble = std::make_shared<FITScolumn<double>>("COL_DOUBLE", tdouble, "", 1);
        //                                        0    1     2     3     4     5    6     7     8    9    10    11
        for (double value : std::vector<double>{-1.5, -0.25, 0.25, 0.75, 4.5, -2.3, 3.14, 2.71, 0.0, 1.1, -3.3, 42.0})
        {
            colDouble->push_back(value);
        }
        table.InsertColumn(colDouble);

        auto colString = std::make_shared<FITScolumn<std::string>>("COL_STR", tstring, "", 1);
        //                                                        0        1       2        3        4          5       6      7        8       9        10       11
        for (const std::string& value : std::vector<std::string>{"alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta", "iota", "kappa", "lambda", "mu"})
        {
            colString->push_back(value);
        }
        table.InsertColumn(colString);

        return table;
    }


    std::vector<size_t> IndicesOf(const RowSet& rows)
    {
        const auto& raw = rows.indices();
        return std::vector<size_t>(raw.begin(), raw.end());
    }

} // namespace

TEST(RowSetBuilderTest, FiltersAndBuild)
{
    FITStable table = CreateSampleTable();

    RowSet positive = table.select<int32_t>("COL_INT").gt(0).build();
    EXPECT_EQ((std::vector<size_t>{3,4,5,6,7,8,11}), IndicesOf(positive));

    RowSet positive_ = table.filter<int32_t>("COL_INT") > 1;
    EXPECT_EQ(IndicesOf(positive), IndicesOf(positive_));

    RowSet geDoubles = table.select<double>("COL_DOUBLE").ge(-0.5).build();
    EXPECT_EQ((std::vector<size_t>{1,2,3,4,6,7,8,9,11}), IndicesOf(geDoubles));

    RowSet leDoubles = table.select<double>("COL_DOUBLE").le(1.0).build();
    EXPECT_EQ((std::vector<size_t>{0,1,2,3,5,8,10}), IndicesOf(leDoubles));

    RowSet midDoubles = table.select<double>("COL_DOUBLE").between(-0.5, 1.0).build();
    EXPECT_EQ((std::vector<size_t>{1,2,3,8}), IndicesOf(midDoubles)) ;

    RowSet custom = table.select<int32_t>("COL_INT")
                         .custom([](int32_t value, size_t) { return value % 3 == 0; })
                         .build();
    EXPECT_EQ((std::vector<size_t>{2,4,5,6,7}), IndicesOf(custom));
}

TEST(RowSetTest, SetOperations)
{
    FITStable table = CreateSampleTable();

    RowSet positives = table.select<int32_t>("COL_INT").gt(0).build();
    RowSet negatives = table.select<int32_t>("COL_INT").lt(0).build();

    EXPECT_TRUE(positives.intersected(negatives).empty());

    auto unionIdx = positives.united(negatives).indices();
    EXPECT_EQ(table.nrows() - 1, unionIdx.size()); // zero row excluded

    auto diffIdx = positives.subtracted(negatives).indices();
    EXPECT_EQ(IndicesOf(positives), diffIdx);
}

TEST(ColumnViewTest, MutateSelectionOnly)
{
    FITStable table = CreateSampleTable();

    RowSet rows = table.select<double>("COL_DOUBLE").between(0.0, 5.0).build();
    auto before = table.column<double>("COL_DOUBLE").data();

    table.column<double>("COL_DOUBLE").on(rows).add(1.0);

    const auto& after = table.column<double>("COL_DOUBLE").data();
    ASSERT_EQ(before.size(), after.size());

    for(size_t i = 0; i < after.size(); ++i)
    {
        if(std::find(rows.indices().begin(), rows.indices().end(), i) != rows.indices().end())
            EXPECT_DOUBLE_EQ(before[i] + 1.0, after[i]);
        else
            EXPECT_DOUBLE_EQ(before[i], after[i]);
    }
}

TEST(ColumnHandleTest, WhereChainAppliesAcrossColumns)
{
    FITStable table = CreateSampleTable();

    table["COL_DOUBLE"].where<int32_t>("COL_INT").ge(3).set<double>(99.0);

    const auto& ints = table.column<int32_t>("COL_INT").data();
    const auto& doubles = table.column<double>("COL_DOUBLE").data();

    ASSERT_EQ(ints.size(), doubles.size());
    for(size_t i = 0; i < ints.size(); ++i)
    {
        if(ints[i] >= 3)
            EXPECT_DOUBLE_EQ(99.0, doubles[i]);
        else
            EXPECT_DOUBLE_EQ(CreateSampleTable().column<double>("COL_DOUBLE").data()[i], doubles[i]);
    }
}

TEST(ColumnViewTest, SortAscendingPreservesAlignment)
{
    FITStable table = CreateSampleTable();

    auto intsBefore = table.column<int32_t>("COL_INT").data();
    auto doublesBefore = table.column<double>("COL_DOUBLE").data();

    std::vector<std::tuple<int32_t,double,size_t>> expected;
    expected.reserve(intsBefore.size());
    for(size_t i = 0; i < intsBefore.size(); ++i)
        expected.emplace_back(intsBefore[i], doublesBefore[i], i);

    std::stable_sort(expected.begin(), expected.end(),
        [](const auto& lhs, const auto& rhs)
        {
            if(std::get<0>(lhs) != std::get<0>(rhs))
                return std::get<0>(lhs) < std::get<0>(rhs);
            return std::get<2>(lhs) < std::get<2>(rhs);
        });

    table.column<int32_t>("COL_INT").sortAscending();

    const auto& intsAfter = table.column<int32_t>("COL_INT").data();
    const auto& doublesAfter = table.column<double>("COL_DOUBLE").data();

    for(size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(std::get<0>(expected[i]), intsAfter[i]);
        EXPECT_DOUBLE_EQ(std::get<1>(expected[i]), doublesAfter[i]);
    }
}

TEST(RowSetBuilderTest, AllComparatorsAndBetweenEdges)
{
    FITStable table = CreateSampleTable();

    // eq / ne on integers
    RowSet eq3 = table.select<int32_t>("COL_INT").eq(3).build();
    EXPECT_EQ((std::vector<size_t>{6,7}), IndicesOf(eq3));

    RowSet ne3 = table.select<int32_t>("COL_INT").ne(3).build();
    EXPECT_EQ((std::vector<size_t>{0,1,2,3,4,5,8,9,10,11}), IndicesOf(ne3));

    // lt / le / gt / ge on doubles
    RowSet ltZero = table.select<double>("COL_DOUBLE").lt(0.0).build();
    EXPECT_EQ((std::vector<size_t>{0,1,5, 10}), IndicesOf(ltZero));

    RowSet leZero = table.select<double>("COL_DOUBLE").le(0.0).build();
    EXPECT_EQ((std::vector<size_t>{0,1,5,8,10}), IndicesOf(leZero)); // note: 2 and 8 are 0.25 and 0.0 respectively; sorted in RowSet

    RowSet gtPi = table.select<double>("COL_DOUBLE").gt(3.14).build();
    EXPECT_EQ((std::vector<size_t>{4,11}), IndicesOf(gtPi));

    RowSet gePi = table.select<double>("COL_DOUBLE").ge(3.14).build();
    EXPECT_EQ((std::vector<size_t>{4,6,11}), IndicesOf(gePi));

    // between is inclusive; check boundaries hit
    RowSet betweenExact = table.select<double>("COL_DOUBLE").between(0.25, 3.14).build();
    EXPECT_EQ((std::vector<size_t>{2,3,6,7,9}), IndicesOf(betweenExact)); // includes 0.25 and 3.14

    // custom predicate using row index
    RowSet customIdxEven = table.select<int32_t>("COL_INT")
                            .custom([](int32_t, size_t idx){ return idx % 2 == 0; })
                            .build();
    std::vector<size_t> expectedEven;
    for(size_t i=0;i<table.nrows();++i) if(i%2==0) expectedEven.push_back(i);
    EXPECT_EQ(expectedEven, IndicesOf(customIdxEven));
}

TEST(RowSetBuilderTest, AllComparatorsBySymbol)
{
    FITStable table = CreateSampleTable();

    // eq / ne on integers
    RowSet eq3 = table.filter<int32_t>("COL_INT") == 3;
    EXPECT_EQ((std::vector<size_t>{6,7}), IndicesOf(eq3));

    RowSet ne3 = table.filter<int32_t>("COL_INT") != 3;
    EXPECT_EQ((std::vector<size_t>{0,1,2,3,4,5,8,9,10,11}), IndicesOf(ne3));

    // lt / le / gt / ge on doubles
    RowSet ltZero = table.filter<double>("COL_DOUBLE") < 0.0;
    EXPECT_EQ((std::vector<size_t>{0,1,5,10}), IndicesOf(ltZero));

    RowSet leZero = table.filter<double>("COL_DOUBLE") <= 0.0;
    EXPECT_EQ((std::vector<size_t>{0,1,5,8,10}), IndicesOf(leZero)); // note: 2 and 8 are 0.25 and 0.0 respectively; sorted in RowSet

    RowSet gtPi = table.filter<double>("COL_DOUBLE") > 3.14;
    EXPECT_EQ((std::vector<size_t>{4,11}), IndicesOf(gtPi));

    RowSet gePi = table.filter<double>("COL_DOUBLE") >= 3.14;
    EXPECT_EQ((std::vector<size_t>{4,6,11}), IndicesOf(gePi));

    // between is inclusive; check boundaries hit
    RowSet betweenExact = table.filter<double>("COL_DOUBLE") >  0.25 && table.filter<double>("COL_DOUBLE") < 3.14;
    EXPECT_EQ((std::vector<size_t>{3,7,9}), IndicesOf(betweenExact)); // includes 0.25 and 3.14

    RowSet between = table.filter<double>("COL_DOUBLE") >=  0.25 && table.filter<double>("COL_DOUBLE") <= 3.14;
    EXPECT_EQ((std::vector<size_t>{2,3,6,7,9}), IndicesOf(between)); // includes 0.25 and 3.14
}

TEST(ColumnFilterExprTest, OperatorsMirrorBuilder)
{
    FITStable table = CreateSampleTable();

    // operator==
    RowSet eqBeta = table.filter<std::string>("COL_STR") == std::string("beta");
    EXPECT_EQ((std::vector<size_t>{1}), IndicesOf(eqBeta));

    // operator!=
    RowSet neMu = table.filter<std::string>("COL_STR") != std::string("mu");
    std::vector<size_t> expectNeMu;
    for(size_t i=0;i<table.nrows();++i) if(i != 11) expectNeMu.push_back(i);
    EXPECT_EQ(expectNeMu, IndicesOf(neMu));

    // operator< / <= on ints
    RowSet ltMinus1 = table.filter<int32_t>("COL_INT") < -1;
    EXPECT_EQ((std::vector<size_t>{0,9,10}), IndicesOf(ltMinus1));

    RowSet leMinus1 = table.filter<int32_t>("COL_INT") <= -1;
    EXPECT_EQ((std::vector<size_t>{0,1,9,10}), IndicesOf(leMinus1));

    // operator> / >= on doubles
    RowSet gtTwo = table.filter<double>("COL_DOUBLE") > 2.0;
    EXPECT_EQ((std::vector<size_t>{4,6,7,11}), IndicesOf(gtTwo));

    RowSet geTwo = table.filter<double>("COL_DOUBLE") >= 2.0;
    EXPECT_EQ((std::vector<size_t>{4,6,7,11}), IndicesOf(geTwo));

    // between
    RowSet bet = table.filter<int32_t>("COL_INT").between(-1, 4);
    EXPECT_EQ((std::vector<size_t>{1,2,3,6,7}), IndicesOf(bet));
}

TEST(RowSetTest, SetOperationsExtended)
{
    FITStable table = CreateSampleTable();

    RowSet positives = table.select<int32_t>("COL_INT").gt(0).build();
    RowSet negatives = table.select<int32_t>("COL_INT").lt(0).build();
    RowSet zeros     = table.select<int32_t>("COL_INT").eq(0).build();

    // Disjoint sets
    EXPECT_TRUE(positives.intersected(negatives).empty());
    EXPECT_TRUE(positives.intersected(zeros).empty());
    EXPECT_TRUE(negatives.intersected(zeros).empty());

    // Union covers all rows
    auto unionAll = positives.united(negatives).united(zeros).indices();
    EXPECT_EQ(table.nrows(), unionAll.size());

    // Subtraction symmetry
    EXPECT_EQ(IndicesOf(positives), positives.subtracted(negatives.united(zeros)).indices());
    EXPECT_EQ(IndicesOf(negatives), negatives.subtracted(positives.united(zeros)).indices());
}

TEST(ColumnViewTest, ClearSelectionAndApply)
{
    FITStable table = CreateSampleTable();

    RowSet rows = table.select<int32_t>("COL_INT").between(0, 5).build();
    auto before = table.column<int32_t>("COL_INT").data();

    // Apply only to selection
    table.column<int32_t>("COL_INT").on(rows).apply([](int32_t& v, size_t){ v += 10; });

    auto mid = table.column<int32_t>("COL_INT").data();
    for(size_t i=0;i<mid.size();++i)
    {
        bool in = std::binary_search(rows.indices().begin(), rows.indices().end(), i);
        EXPECT_EQ(before[i] + (in ? 10 : 0), mid[i]);
    }

    // Clear selection and apply to all
    table.column<int32_t>("COL_INT").clearSelection().apply([](int32_t& v, size_t){ v -= 1; });

    auto after = table.column<int32_t>("COL_INT").data();
    for(size_t i=0;i<after.size();++i)
    {
        int32_t expected = before[i] + (std::binary_search(rows.indices().begin(), rows.indices().end(), i) ? 9 : -1);
        EXPECT_EQ(expected, after[i]);
    }
}

TEST(ColumnViewTest, SortDescendingPreservesAlignment)
{
    FITStable table = CreateSampleTable();

    auto intsBefore = table.column<int32_t>("COL_INT").data();
    auto doublesBefore = table.column<double>("COL_DOUBLE").data();

    // Build expected stable sort by descending int values, tie-breaker by original index
    std::vector<std::tuple<int32_t,double,size_t>> expected;
    expected.reserve(intsBefore.size());
    for(size_t i = 0; i < intsBefore.size(); ++i)
        expected.emplace_back(intsBefore[i], doublesBefore[i], i);

    std::stable_sort(expected.begin(), expected.end(),
        [](const auto& lhs, const auto& rhs)
        {
            if(std::get<0>(lhs) != std::get<0>(rhs))
                return std::get<0>(lhs) > std::get<0>(rhs);
            return std::get<2>(lhs) < std::get<2>(rhs);
        });

    table.column<int32_t>("COL_INT").sortDescending();

    const auto& intsAfter = table.column<int32_t>("COL_INT").data();
    const auto& doublesAfter = table.column<double>("COL_DOUBLE").data();

    for(size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(std::get<0>(expected[i]), intsAfter[i]);
        EXPECT_DOUBLE_EQ(std::get<1>(expected[i]), doublesAfter[i]);
    }
}

TEST(ColumnHandleTest, WhereChainApplyAndArithmetic)
{
    FITStable table = CreateSampleTable();

    // Use custom predicate on COL_INT: multiples of 4
    table["COL_INT"].where<int32_t>("COL_INT")
        .custom([](int32_t v, size_t /*idx*/){ return (v % 4) == 0; }) // accept idx to match predicate arity
        .add<int32_t>(1)
        .mul<int32_t>(2);

    // Capture baseline once; avoid accessing temporaries
    FITStable base = CreateSampleTable();
    std::vector<int32_t> baseInts = base.column<int32_t>("COL_INT").data();

    const auto& ints = table.column<int32_t>("COL_INT").data();
    for(size_t i=0;i<ints.size();++i)
    {
        int32_t orig = baseInts[i];
        if(orig % 4 == 0)
            EXPECT_EQ((orig + 1) * 2, ints[i]);
        else
            EXPECT_EQ(orig, ints[i]);
    }

    // Apply double ops on subset where int >= 0 and <= 3
    FITStable table2 = CreateSampleTable();
    table2["COL_DOUBLE"]
        .where<int32_t>("COL_INT").between(0, 3)
        .sub<double>(0.5)
        .div<double>(0.5)
        .apply<double>([](double& v, size_t){ v = v + 2.0; });

    // Capture baselines once
    FITStable base2 = CreateSampleTable();
    std::vector<double> baseDoubles = base2.column<double>("COL_DOUBLE").data();
    std::vector<int32_t> baseInts2  = base2.column<int32_t>("COL_INT").data();

    const auto& doubles = table2.column<double>("COL_DOUBLE").data();
    for(size_t i=0;i<doubles.size();++i)
    {
        if(baseInts2[i] >= 0 && baseInts2[i] <= 3)
        {
            double expected = ((baseDoubles[i] - 0.5) / 0.5) + 2.0;
            EXPECT_DOUBLE_EQ(expected, doubles[i]);
        }
        else
        {
            EXPECT_DOUBLE_EQ(baseDoubles[i], doubles[i]);
        }
    }
}

TEST(RowSetBuilderTest, LogicalCombinatorsAndOr)
{
    FITStable table = CreateSampleTable();

    // Using && between two RowSet results from ColumnFilterExpr
    RowSet between1 = table.filter<double>("COL_DOUBLE") > 0.25 && table.filter<double>("COL_DOUBLE") < 3.14;
    RowSet between2 = (table.filter<double>("COL_DOUBLE") > 0.25) && (table.filter<double>("COL_DOUBLE") < 3.14);
    EXPECT_EQ(IndicesOf(between1), IndicesOf(between2));

    // Compare with explicit between builder for clarity (exclusive ends above)
    RowSet inclusive = table.select<double>("COL_DOUBLE").between(0.25, 3.14).build();
    // exclusive should be inclusive minus endpoints 0.25 (idx=2) and 3.14 (idx=6)
    std::vector<size_t> expectedExclusive = inclusive.subtracted(table.filter<double>("COL_DOUBLE") == 0.25)
                                            .subtracted(table.filter<double>("COL_DOUBLE") == 3.14).indices();
    EXPECT_EQ(expectedExclusive, IndicesOf(between1));

    // Using || to union two conditions (ints <= -5 OR ints >= 10)
    RowSet lower = table.filter<int32_t>("COL_INT") <= (-1);
    EXPECT_EQ((std::vector<size_t>{0,1,9,10}), IndicesOf(lower)); // sorted automatically: {0,4,5,8,10,11}

    // Using || to union two conditions (ints <= -5 OR ints >= 10)
    RowSet upper = table.filter<int32_t>("COL_INT") >= 10;
    EXPECT_EQ((std::vector<size_t>{5,8,11}), IndicesOf(upper)); // sorted automatically: {0,4,5,8,10,11}

    RowSet extremes = (table.filter<int32_t>("COL_INT") <= -1 ) || (table.filter<int32_t>("COL_INT") >= 10);
    EXPECT_EQ((std::vector<size_t>{0,1,5,8,9,10,11}), IndicesOf(extremes)); // sorted automatically: {0,4,5,8,10,11}
}

TEST(RowSetBuilderTest, CustomUnaryPredicateSupported)
{
    FITStable table = CreateSampleTable();
    RowSet rs = table.select<int32_t>("COL_INT")
                     .custom([](const int32_t v){ return v < 0; })
                     .build();
    EXPECT_EQ((std::vector<size_t>{0,1,9,10}), IndicesOf(rs));
}

TEST(FITStableEdgeTest, EmptyAndSingleRowTables)
{
    // Empty
    FITStable empty;
    auto colE = std::make_shared<FITScolumn<int32_t>>("E", tint, "", 1);
    empty.InsertColumn(colE);
    EXPECT_EQ(0u, empty.nrows());
    EXPECT_TRUE(empty.select<int32_t>("E").gt(0).build().empty());

    // Single
    FITStable single;
    auto colS = std::make_shared<FITScolumn<int32_t>>("S", tint, "", 1);
    colS->push_back(5);
    single.InsertColumn(colS);
    EXPECT_EQ((std::vector<size_t>{0}), IndicesOf(single.select<int32_t>("S").ge(5).build()));
    EXPECT_TRUE(single.select<int32_t>("S").lt(0).build().empty());
}

TEST(FITStableTypeTest, WrongScalarTypeThrows)
{
    FITStable table = CreateSampleTable();
    EXPECT_THROW(table.select<double>("COL_INT").build(), std::bad_cast);
    EXPECT_THROW((void)table.column<std::string>("COL_DOUBLE"), std::bad_cast);
}

TEST(FITStableOrderTest, ReorderRowsValidation)
{
    FITStable table = CreateSampleTable();
    // Wrong size
    EXPECT_THROW(table.reorderRows(std::vector<size_t>{0,1}), std::logic_error);
    // Out of range
    std::vector<size_t> bad(table.nrows());
    std::iota(bad.begin(), bad.end(), 0);
    bad.back() = table.nrows(); // invalid index
    EXPECT_THROW(table.reorderRows(bad), std::logic_error);
    // Duplicate indices
    std::vector<size_t> dup(table.nrows());
    std::iota(dup.begin(), dup.end(), 0);
    dup[1] = 0;
    EXPECT_THROW(table.reorderRows(dup), std::logic_error);
}

TEST(RowSetTest, EmptyAndFullSetOperations)
{
    FITStable table = CreateSampleTable();
    RowSet empty;
    RowSet all = table.select<int32_t>("COL_INT").ge(std::numeric_limits<int32_t>::min()).build();

    EXPECT_TRUE(empty.united(all).indices() == all.indices());
    EXPECT_TRUE(empty.intersected(all).empty());
    EXPECT_TRUE(all.subtracted(empty).indices() == all.indices());
}

TEST(ColumnViewTest, StatisticalComputations)
{

    const size_t N = 1000000;
    float dN = static_cast<double>(N);
    std::default_random_engine re;
    re.seed(123456); // deterministic for test
    std::normal_distribution<float> norm(0.0, 1.0);   // normal( mean=0, sigma=1 )
    std::uniform_real_distribution<float> unif(0, 1);   // normal( mean=0, sigma=1 )

    FITScolumn<float> col("NORM", tfloat, "", 1);
    FITScolumn<bool> flag("FLAG", tlogical, "", 1);
    
    float sum = 0.0f;
    float sumsq = 0.0f;
    float sum3  = 0.0f;
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::lowest();

    float f_sum = 0.0f;
    float f_sumsq = 0.0f;
    float f_sum3  = 0.0f;
    float f_min = std::numeric_limits<float>::max();
    float f_max = std::numeric_limits<float>::lowest();
    float f_N=0.0f;
    
    for (size_t i = 0; i < N; ++i)
    {
        float sample = norm(re);
        float u = unif(re);
        
        
        sum += sample;
        sumsq += sample * sample;
        sum3  += sample * sample * sample;

        min = (sample < min) ? sample : min;
        max = (sample > max) ? sample : max;

        if(u < 0.5f)
        {
            f_sum   += sample;
            f_sumsq += sample * sample;
            f_sum3  += sample * sample * sample;
            
            f_min = (sample < f_min) ? sample : f_min;
            f_max = (sample > f_max) ? sample : f_max;
            f_N ++;
        }

        col.push_back(sample);
        flag.push_back(u < 0.5f); // 10% true
    }

    double mean = sum / dN;
    double variance = (sumsq /dN) - (mean * mean);
    double stddev = std::sqrt(variance);
    double rms = std::sqrt(sumsq / dN);
    double skewness = ((sum3 / dN) - 3 * mean * variance - mean * mean * mean) / (stddev * stddev * stddev);

    double f_mean = f_sum / f_N;
    double f_variance = (f_sumsq /f_N) - (f_mean * f_mean);
    double f_stddev = std::sqrt(f_variance);
    double f_rms = std::sqrt(f_sumsq / f_N);
    double f_skewness = ((f_sum3 / f_N) - 3 * f_mean * f_variance - f_mean * f_mean * f_mean) / (f_stddev * f_stddev * f_stddev);

    FITStable table;
    table.InsertColumn(std::make_shared< FITScolumn<float> >(col));
    table.InsertColumn(std::make_shared< FITScolumn<bool> >(flag));

    EXPECT_NEAR(table.column<float>("NORM").mean()    , mean, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").min()     , min, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").max()     , max, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").variance(), variance, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").rmse()    , stddev, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").rms()     , rms, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").skewness(), skewness, 1e-5);

    auto flaggedRows = table.select<bool>("FLAG").eq(true).build();

    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).mean()    , f_mean, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).min()     , f_min, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).max()     , f_max, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).variance(), f_variance, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).rmse()    , f_stddev, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).rms()     , f_rms, 1e-6);
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).skewness(), f_skewness, 1e-5);
}

TEST(ColumnViewTest, StatisticalComputations_inParralel)
{

    const size_t N = 1000000;
    float dN = static_cast<double>(N);
    std::default_random_engine re;
    re.seed(123456); // deterministic for test
    std::normal_distribution<float> norm(0.0, 1.0);   // normal( mean=0, sigma=1 )
    std::uniform_real_distribution<float> unif(0, 1);   // normal( mean=0, sigma=1 )

    FITScolumn<float> col("NORM", tfloat, "", 1);
    FITScolumn<bool> flag("FLAG", tlogical, "", 1);
    
    float sum = 0.0f;
    float sumsq = 0.0f;
    float sum3  = 0.0f;
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::lowest();

    float f_sum = 0.0f;
    float f_sumsq = 0.0f;
    float f_sum3  = 0.0f;
    float f_min = std::numeric_limits<float>::max();
    float f_max = std::numeric_limits<float>::lowest();
    float f_N=0.0f;
    
    for (size_t i = 0; i < N; ++i)
    {
        float sample = norm(re);
        float u = unif(re);
        
        
        sum += sample;
        sumsq += sample * sample;
        sum3  += sample * sample * sample;

        min = (sample < min) ? sample : min;
        max = (sample > max) ? sample : max;

        if(u < 0.5f)
        {
            f_sum   += sample;
            f_sumsq += sample * sample;
            f_sum3  += sample * sample * sample;
            
            f_min = (sample < f_min) ? sample : f_min;
            f_max = (sample > f_max) ? sample : f_max;
            f_N ++;
        }

        col.push_back(sample);
        flag.push_back(u < 0.5f); // 10% true
    }

    double mean = sum / dN;
    double variance = (sumsq /dN) - (mean * mean);
    double stddev = std::sqrt(variance);
    double rms = std::sqrt(sumsq / dN);
    double skewness = ((sum3 / dN) - 3 * mean * variance - mean * mean * mean) / (stddev * stddev * stddev);

    double f_mean = f_sum / f_N;
    double f_variance = (f_sumsq /f_N) - (f_mean * f_mean);
    double f_stddev = std::sqrt(f_variance);
    double f_rms = std::sqrt(f_sumsq / f_N);
    double f_skewness = ((f_sum3 / f_N) - 3 * f_mean * f_variance - f_mean * f_mean * f_mean) / (f_stddev * f_stddev * f_stddev);

    FITStable table;
    table.InsertColumn(std::make_shared< FITScolumn<float> >(col));
    table.InsertColumn(std::make_shared< FITScolumn<bool> >(flag));

    std::thread t1([&](){

        EXPECT_NEAR(table.column<float>("NORM").mean()    , mean, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").min()     , min, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").max()     , max, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").variance(), variance, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").rmse()    , stddev, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").rms()     , rms, 1e-6);
    });

    std::thread t2([&](){
        auto flaggedRows = table.select<bool>("FLAG").eq(true).build();

        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).mean()    , f_mean, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).min()     , f_min, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).max()     , f_max, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).variance(), f_variance, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).rmse()    , f_stddev, 1e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).rms()     , f_rms, 1e-6);
    });

    std::thread t3([&](){
        // just to increase concurrency
        for(int i=0;i<100000;++i)
        {
            float sample = norm(re);
            table.push_back<float>("NORM", sample);
        }
    });

    t1.join();
    t2.join();
    t3.join();

}