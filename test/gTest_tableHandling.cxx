#include <gtest/gtest.h>
#include <DSTfits/FITStable.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <tuple>
#include <random>
#include <thread>
#include <sstream>
#include <string>
#include <limits>
#include <cmath>

#include "gTest_locale.h"


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

TEST(ToCSVTest, ExportsScalarAndVectorCells)
{
    FITStable table;

    auto colScalar = std::make_shared<FITScolumn<int32_t>>("SCALAR", tint, "px", 1);
    colScalar->push_back(1);
    colScalar->push_back(2);
    table.InsertColumn(colScalar);

    auto colVec = std::make_shared<FITScolumn<FITSform::int32Vector>>("VEC", tint, "adu", 2);
    colVec->push_back(FITSform::int32Vector{1, 2, 3});
    colVec->push_back(FITSform::int32Vector{42,8,5});
    table.InsertColumn(colVec);

    auto colStr = std::make_shared<FITScolumn< std::string> >("STR", tstring, "", 3);
    colStr->push_back(std::string("hello"));
    colStr->push_back(std::string("plain"));
    table.InsertColumn(colStr);

    std::ostringstream oss;
    table.toCSV(oss, '\t');

    std::cout<<oss.str();

    const std::string expected =
        "SCALAR\tVEC\tSTR\n"
        "[px]\t[adu]\t\n"
        "1\t{1;2;3}\thello\n"
        "2\t{42;8;5}\tplain\n";

    EXPECT_EQ(expected, oss.str());
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
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).variance(), f_variance, 1e-5);
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).rmse()    , f_stddev, 1e-5);
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).rms()     , f_rms, 1e-5);
    EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).skewness(), f_skewness, 1e-5);
}

TEST(ColumnViewTest, StatisticalComputations_inParralel)
{

    const size_t N = 1000000;
    float dN = static_cast<double>(N);
    std::default_random_engine re;
    re.seed(12345678910111213); // deterministic for test
    std::normal_distribution<float> norm(0.0, 1.0);   // normal( mean=0, sigma=1 )
    std::uniform_real_distribution<float> unif(0, 1);   // normal( mean=0, sigma=1 )

    FITScolumn<float> col("NORM", tfloat, "", 1);
    FITScolumn<bool> flag("FLAG", tlogical, "", 1);
    
    float sum = 0.0f;
    float sumsq = 0.0f;
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::lowest();

    float f_sum = 0.0f;
    float f_sumsq = 0.0f;
    float f_min = std::numeric_limits<float>::max();
    float f_max = std::numeric_limits<float>::lowest();
    float f_N=0.0f;
    
    for (size_t i = 0; i < N; ++i)
    {
        float sample = norm(re);
        float u = unif(re);
        
        
        sum += sample;
        sumsq += sample * sample;

        min = (sample < min) ? sample : min;
        max = (sample > max) ? sample : max;

        if(u < 0.5f)
        {
            f_sum   += sample;
            f_sumsq += sample * sample;
            
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

    double f_mean = f_sum / f_N;
    double f_variance = (f_sumsq /f_N) - (f_mean * f_mean);
    double f_stddev = std::sqrt(f_variance);
    double f_rms = std::sqrt(f_sumsq / f_N);

    FITStable table;
    table.InsertColumn(std::make_shared< FITScolumn<float> >(col));
    table.InsertColumn(std::make_shared< FITScolumn<bool> >(flag));

    std::thread t1([&](){

        EXPECT_NEAR(table.column<float>("NORM").mean()    , mean, 5e-6);
        EXPECT_NEAR(table.column<float>("NORM").min()     , min, 5e-6);
        EXPECT_NEAR(table.column<float>("NORM").max()     , max, 5e-6);
        EXPECT_NEAR(table.column<float>("NORM").variance(), variance, 5e-5);
        EXPECT_NEAR(table.column<float>("NORM").rmse()    , stddev, 5e-5);
        EXPECT_NEAR(table.column<float>("NORM").rms()     , rms, 5e-5);
    });

    std::thread t2([&](){
        auto flaggedRows = table.select<bool>("FLAG").eq(true).build();

        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).mean()    , f_mean, 5e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).min()     , f_min, 5e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).max()     , f_max, 5e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).variance(), f_variance, 5e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).rmse()    , f_stddev, 5e-6);
        EXPECT_NEAR(table.column<float>("NORM").on(flaggedRows).rms()     , f_rms, 5e-6);
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

// Add these tests after the existing tests

TEST(RowIteratorTest, BasicIteration)
{
    FITStable table = CreateSampleTable();
    
    size_t count = 0;
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        EXPECT_EQ(count, it.index());
        EXPECT_EQ(count, *it);
        ++count;
    }
    EXPECT_EQ(table.nrows(), count);
}

TEST(RowIteratorTest, RangeBasedForLoop)
{
    FITStable table = CreateSampleTable();
    
    std::vector<size_t> indices;
    for(auto it : table)
    {
        indices.push_back(it);
    }
    
    EXPECT_EQ(table.nrows(), indices.size());
    for(size_t i = 0; i < indices.size(); ++i)
    {
        EXPECT_EQ(i, indices[i]);
    }
}

TEST(RowIteratorTest, AccessMultipleColumns)
{
    FITStable table = CreateSampleTable();
    
    // Reference data
    auto ints = table.column<int32_t>("COL_INT").data();
    auto doubles = table.column<double>("COL_DOUBLE").data();
    auto strings = table.column<std::string>("COL_STR").data();
    
    size_t idx = 0;
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        EXPECT_EQ(ints[idx], it.at<int32_t>("COL_INT"));
        EXPECT_DOUBLE_EQ(doubles[idx], it.at<double>("COL_DOUBLE"));
        EXPECT_EQ(strings[idx], it.at<std::string>("COL_STR"));
        ++idx;
    }
    EXPECT_EQ(table.nrows(), idx);
}

TEST(RowIteratorTest, ModifyThroughIterator)
{
    FITStable table = CreateSampleTable();
    
    // Multiply all ints by 2
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        it.at<int32_t>("COL_INT") *= 2;
    }
    
    // Verify changes
    auto original = CreateSampleTable();
    auto originalInts = original.column<int32_t>("COL_INT").data();
    auto modifiedInts = table.column<int32_t>("COL_INT").data();
    
    ASSERT_EQ(originalInts.size(), modifiedInts.size());
    for(size_t i = 0; i < originalInts.size(); ++i)
    {
        EXPECT_EQ(originalInts[i] * 2, modifiedInts[i]);
    }
}

TEST(RowIteratorTest, ConditionalAccess)
{
    FITStable table = CreateSampleTable();
    
    std::vector<double> positiveDoubles;
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        int32_t intVal = it.at<int32_t>("COL_INT");
        if(intVal > 0)
        {
            positiveDoubles.push_back(it.at<double>("COL_DOUBLE"));
        }
    }
    
    // Verify against filter-based approach
    auto positiveRows = table.select<int32_t>("COL_INT").gt(0).build();
    EXPECT_EQ(positiveRows.size(), positiveDoubles.size());
}

TEST(RowIteratorTest, IteratorCaching)
{
    FITStable table = CreateSampleTable();
    
    // Access same column multiple times per row - should use cached iterator
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        double d1 = it.at<double>("COL_DOUBLE");
        double d2 = it.at<double>("COL_DOUBLE");
        EXPECT_DOUBLE_EQ(d1, d2);
        
        // Access different column
        int32_t i1 = it.at<int32_t>("COL_INT");
        
        // Access first column again
        double d3 = it.at<double>("COL_DOUBLE");
        EXPECT_DOUBLE_EQ(d1, d3);
    }
}

TEST(RowIteratorTest, MixedAccessPatterns)
{
    FITStable table = CreateSampleTable();
    
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        size_t idx = it.index();
        
        // Alternating access pattern
        if(idx % 2 == 0)
        {
            it.at<int32_t>("COL_INT");
        }
        else
        {
            it.at<double>("COL_DOUBLE");
        }
        
        // Always access string
        it.at<std::string>("COL_STR");
    }
    // Just verify it completes without error
    SUCCEED();
}

TEST(RowIteratorTest, PartialIteration)
{
    FITStable table = CreateSampleTable();
    
    auto it = table.begin();
    EXPECT_EQ(0u, it.index());
    
    ++it;
    EXPECT_EQ(1u, it.index());
    EXPECT_EQ(-1, it.at<int32_t>("COL_INT"));
    
    ++it;
    ++it;
    EXPECT_EQ(3u, it.index());
    EXPECT_EQ(4, it.at<int32_t>("COL_INT"));
    
    // Skip to near end
    while(it.index() < table.nrows() - 1)
        ++it;
    
    EXPECT_EQ(table.nrows() - 1, it.index());
    EXPECT_EQ(10, it.at<int32_t>("COL_INT"));
}

TEST(RowIteratorTest, PostIncrement)
{
    FITStable table = CreateSampleTable();
    
    auto it = table.begin();
    auto old = it++;
    
    EXPECT_EQ(0u, old.index());
    EXPECT_EQ(1u, it.index());
    
    EXPECT_EQ(-5, old.at<int32_t>("COL_INT"));
    EXPECT_EQ(-1, it.at<int32_t>("COL_INT"));
}

TEST(RowIteratorTest, EqualityComparison)
{
    FITStable table = CreateSampleTable();
    
    auto it1 = table.begin();
    auto it2 = table.begin();
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);
    
    ++it2;
    EXPECT_FALSE(it1 == it2);
    EXPECT_TRUE(it1 != it2);
    
    ++it1;
    EXPECT_TRUE(it1 == it2);
    
    auto end1 = table.end();
    auto end2 = table.end();
    EXPECT_TRUE(end1 == end2);
}

TEST(RowIteratorTest, EmptyTable)
{
    FITStable table;
    auto col = std::make_shared<FITScolumn<int32_t>>("EMPTY", tint, "", 1);
    table.InsertColumn(col);
    
    EXPECT_EQ(table.begin(), table.end());
    
    // Should not enter loop
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        FAIL() << "Should not iterate over empty table";
    }
}

TEST(RowIteratorTest, SingleRowTable)
{
    FITStable table;
    auto col = std::make_shared<FITScolumn<int32_t>>("SINGLE", tint, "", 1);
    col->push_back(42);
    table.InsertColumn(col);
    
    size_t count = 0;
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        EXPECT_EQ(0u, it.index());
        EXPECT_EQ(42, it.at<int32_t>("SINGLE"));
        ++count;
    }
    EXPECT_EQ(1u, count);
}

TEST(RowIteratorTest, LargeTablePerformance)
{
    FITStable table;
    auto col = std::make_shared<FITScolumn<double>>("LARGE", tdouble, "", 1);
    
    const size_t N = 100000;
    for(size_t i = 0; i < N; ++i)
    {
        col->push_back(static_cast<double>(i));
    }
    table.InsertColumn(col);
    
    // Iterate and verify
    size_t idx = 0;
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        EXPECT_DOUBLE_EQ(static_cast<double>(idx), it.at<double>("LARGE"));
        ++idx;
    }
    EXPECT_EQ(N, idx);
}

TEST(RowIteratorTest, WrongTypeThrows)
{
    FITStable table = CreateSampleTable();
    
    auto it = table.begin();
    EXPECT_THROW(it.at<double>("COL_INT"), std::bad_cast);
    EXPECT_THROW(it.at<int32_t>("COL_DOUBLE"), std::bad_cast);
    EXPECT_THROW(it.at<float>("COL_STR"), std::bad_cast);
}

TEST(RowIteratorTest, InvalidColumnNameThrows)
{
    FITStable table = CreateSampleTable();
    
    auto it = table.begin();
    EXPECT_THROW(it.at<int32_t>("NONEXISTENT"), std::out_of_range);
}

TEST(RowIteratorTest, ThreadSafetyReadOnly)
{
    FITStable table = CreateSampleTable();
    
    std::vector<std::thread> threads;
    std::atomic<bool> failed{false};
    
    for(int t = 0; t < 4; ++t)
    {
        threads.emplace_back([&table, &failed]()
        {
            try
            {
                for(int iter = 0; iter < 1000; ++iter)
                {
                    for(auto it = table.begin(); it != table.end(); ++it)
                    {
                        volatile int32_t i = it.at<int32_t>("COL_INT");
                        volatile double d = it.at<double>("COL_DOUBLE");
                        (void)i; (void)d;
                    }
                }
            }
            catch(...)
            {
                failed = true;
            }
        });
    }
    
    for(auto& t : threads)
        t.join();
    
    EXPECT_FALSE(failed);
}

TEST(RowIteratorTest, CombineWithFilters)
{
    FITStable table = CreateSampleTable();
    
    auto positiveRows = table.select<int32_t>("COL_INT").gt(0).build();
    
    // Manually iterate and check membership
    std::vector<int32_t> positiveVals;
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        size_t idx = it.index();
        if(std::binary_search(positiveRows.indices().begin(), 
                              positiveRows.indices().end(), idx))
        {
            positiveVals.push_back(it.at<int32_t>("COL_INT"));
        }
    }
    
    // All collected values should be positive
    for(int32_t val : positiveVals)
    {
        EXPECT_GT(val, 0);
    }
    EXPECT_EQ(positiveRows.size(), positiveVals.size());
}

TEST(RowIteratorTest, ModifyMultipleColumnsPerRow)
{
    FITStable table = CreateSampleTable();
    
    // Add 10 to ints, subtract 1 from doubles
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        it.at<int32_t>("COL_INT") += 10;
        it.at<double>("COL_DOUBLE") -= 1.0;
    }
    
    // Verify
    auto original = CreateSampleTable();
    for(auto it = table.begin(); it != table.end(); ++it)
    {
        size_t idx = it.index();
        EXPECT_EQ(original.column<int32_t>("COL_INT").data()[idx] + 10,
                  it.at<int32_t>("COL_INT"));
        EXPECT_DOUBLE_EQ(original.column<double>("COL_DOUBLE").data()[idx] - 1.0,
                         it.at<double>("COL_DOUBLE"));
    }
}

TEST(WhereExpressionTest, ComparisonsMirrorTypedApi)
{
    const FITStable table = CreateSampleTable();

    EXPECT_EQ(IndicesOf(table.where("COL_INT == 3")), IndicesOf(table.select<int32_t>("COL_INT").eq(3).build()));
    EXPECT_EQ(IndicesOf(table.where("COL_INT != 3")), IndicesOf(table.select<int32_t>("COL_INT").ne(3).build()));
    EXPECT_EQ(IndicesOf(table.where("COL_INT < 0")),  IndicesOf(table.select<int32_t>("COL_INT").lt(0).build()));
    EXPECT_EQ(IndicesOf(table.where("COL_INT <= 0")), IndicesOf(table.select<int32_t>("COL_INT").le(0).build()));
    EXPECT_EQ(IndicesOf(table.where("COL_DOUBLE > 3.14")),  IndicesOf(table.select<double>("COL_DOUBLE").gt(3.14).build()));
    EXPECT_EQ(IndicesOf(table.where("COL_DOUBLE >= 3.14")), IndicesOf(table.select<double>("COL_DOUBLE").ge(3.14).build()));
    EXPECT_EQ(IndicesOf(table.where("COL_STR == beta")), (std::vector<size_t>{1}));
}

TEST(WhereExpressionTest, SpacingAndValueForms)
{
    const FITStable table = CreateSampleTable();
    const std::vector<size_t> threes{6, 7};

    EXPECT_EQ(IndicesOf(table.where("COL_INT==3")), threes);
    EXPECT_EQ(IndicesOf(table.where("COL_INT=3")), threes);
    EXPECT_EQ(IndicesOf(table.where("  COL_INT   =   3  ")), threes);
    EXPECT_EQ(IndicesOf(table.where("COL_INT == +3")), threes);
    EXPECT_EQ(IndicesOf(table.where("'COL_INT' == 3")), threes);
    EXPECT_EQ(IndicesOf(table.where("COL_INT==-5")), (std::vector<size_t>{0, 9}));
    EXPECT_EQ(IndicesOf(table.where("COL_DOUBLE >= 4.2e1")), (std::vector<size_t>{11}));
}

TEST(WhereExpressionTest, PrecedenceAndGrouping)
{
    const FITStable table = CreateSampleTable();

    EXPECT_EQ(IndicesOf(table.where("COL_INT == 28 || COL_INT == 3 && COL_DOUBLE > 3")), (std::vector<size_t>{6, 8}));
    EXPECT_EQ(IndicesOf(table.where("(COL_INT == 28 || COL_INT == 3) && COL_DOUBLE > 3")), (std::vector<size_t>{6}));
    EXPECT_EQ(IndicesOf(table.where("COL_INT==28||COL_INT==3&&COL_DOUBLE>3")), (std::vector<size_t>{6, 8}));
    EXPECT_EQ(IndicesOf(table.where("((COL_INT == 28))")), (std::vector<size_t>{8}));
}

TEST(WhereExpressionTest, Wildcards)
{
    const FITStable table = CreateSampleTable();

    EXPECT_EQ(IndicesOf(table.where("COL_STR =~ *eta")), (std::vector<size_t>{1, 5, 6, 7}));
    EXPECT_EQ(IndicesOf(table.where("COL_STR=~?eta")), (std::vector<size_t>{1, 5}));
    EXPECT_EQ(IndicesOf(table.where("COL_STR !~ *a")), (std::vector<size_t>{4, 11}));
    EXPECT_EQ(IndicesOf(table.where("COL_STR =~ 'a*'")), (std::vector<size_t>{0}));

    // Outside =~ and !~ a '*' is only a character.
    EXPECT_TRUE(table.where("COL_STR == *eta").empty());
    EXPECT_EQ(table.where("COL_STR != *eta").size(), table.nrows());
}

TEST(WhereExpressionTest, QuotedValuesAreOneToken)
{
    FITStable table;

    auto names = std::make_shared<FITScolumn<std::string>>("NAME", tstring, "", 1);
    for (const std::string& value : std::vector<std::string>{"WD 1", "a&&b", "", "(x)", "A=B", "O'Brien"})
    {
        names->push_back(value);
    }
    table.InsertColumn(names);

    EXPECT_EQ(IndicesOf(table.where("NAME = 'WD 1'")), (std::vector<size_t>{0}));
    EXPECT_EQ(IndicesOf(table.where("NAME='WD 1'")), (std::vector<size_t>{0}));
    EXPECT_EQ(IndicesOf(table.where("NAME == 'a&&b'")), (std::vector<size_t>{1}));
    EXPECT_EQ(IndicesOf(table.where("NAME == ''")), (std::vector<size_t>{2}));
    EXPECT_EQ(IndicesOf(table.where("NAME == '(x)'")), (std::vector<size_t>{3}));
    EXPECT_EQ(IndicesOf(table.where("NAME == 'A=B'")), (std::vector<size_t>{4}));
    EXPECT_EQ(IndicesOf(table.where("NAME == \"O'Brien\"")), (std::vector<size_t>{5}));
}

TEST(WhereExpressionTest, EmptyExpressionSelectsEveryRow)
{
    const FITStable table = CreateSampleTable();

    EXPECT_EQ(table.where("").size(), table.nrows());
    EXPECT_THROW(table.where("   "), std::invalid_argument);
}

TEST(WhereExpressionTest, ValuesFollowColumnType)
{
    FITStable table;

    auto i8 = std::make_shared<FITScolumn<int8_t>>("I8", tsbyte, "", 1);
    for (int8_t value : std::vector<int8_t>{-128, 0, 5})
    {
        i8->push_back(value);
    }
    table.InsertColumn(i8);

    auto u8 = std::make_shared<FITScolumn<uint8_t>>("U8", tbyte, "", 1);
    for (uint8_t value : std::vector<uint8_t>{0, 200, 255})
    {
        u8->push_back(value);
    }
    table.InsertColumn(u8);

    auto i16 = std::make_shared<FITScolumn<int16_t>>("I16", tshort, "", 1);
    for (int16_t value : std::vector<int16_t>{-300, 0, 70})
    {
        i16->push_back(value);
    }
    table.InsertColumn(i16);

    auto flag = std::make_shared<FITScolumn<bool>>("FLAG", tlogical, "", 1);
    for (bool value : std::vector<bool>{true, false, true})
    {
        flag->push_back(value);
    }
    table.InsertColumn(flag);

    EXPECT_EQ(IndicesOf(table.where("I8 == -128")), (std::vector<size_t>{0}));
    EXPECT_EQ(IndicesOf(table.where("U8 > 200")), (std::vector<size_t>{2}));
    EXPECT_EQ(IndicesOf(table.where("I16 >= 0")), (std::vector<size_t>{1, 2}));

    EXPECT_EQ(IndicesOf(table.where("FLAG == T")), (std::vector<size_t>{0, 2}));
    EXPECT_EQ(IndicesOf(table.where("FLAG != true")), (std::vector<size_t>{1}));
    EXPECT_EQ(IndicesOf(table.where("FLAG = 0")), (std::vector<size_t>{1}));

    EXPECT_THROW(table.where("I8 == -129"), std::invalid_argument);
    EXPECT_THROW(table.where("U8 == 256"), std::invalid_argument);
    EXPECT_THROW(table.where("U8 > -1"), std::invalid_argument);
    EXPECT_THROW(table.where("I16 > 70000"), std::invalid_argument);
    EXPECT_THROW(table.where("FLAG < 1"), std::invalid_argument);
    EXPECT_THROW(table.where("FLAG == maybe"), std::invalid_argument);
}

TEST(WhereExpressionTest, ValuesMustFitEntirely)
{
    const FITStable table = CreateSampleTable();

    EXPECT_THROW(table.where("COL_INT >= 1.5"), std::invalid_argument);
    EXPECT_THROW(table.where("COL_INT > 12abc"), std::invalid_argument);
    EXPECT_THROW(table.where("COL_INT > 99999999999"), std::invalid_argument);
    EXPECT_THROW(table.where("COL_INT == +-3"), std::invalid_argument);
    EXPECT_THROW(table.where("COL_DOUBLE > 1.5x"), std::invalid_argument);
    EXPECT_THROW(table.where("COL_DOUBLE > abc"), std::invalid_argument);
    EXPECT_THROW(table.where("COL_DOUBLE > ''"), std::invalid_argument);
    EXPECT_THROW(table.where("COL_DOUBLE > 1e999"), std::invalid_argument);
    EXPECT_THROW(table.where("COL_DOUBLE > nan"), std::invalid_argument);
    EXPECT_THROW(table.where("COL_DOUBLE < inf"), std::invalid_argument);
}

TEST(WhereExpressionTest, MalformedExpressionsThrow)
{
    const FITStable table = CreateSampleTable();

    for (const std::string& expression : std::vector<std::string>{
             "COL_INT",
             "COL_INT >",
             "COL_INT 3 4",
             "COL_INT > )",
             "COL_INT > 0 COL_INT",
             "COL_INT > 0 &&",
             "&& COL_INT > 0",
             "(COL_INT > 0",
             "COL_INT > 0)",
             "()",
             "COL_STR == 'beta",
             "COL_INT =~ 3*"})
    {
        EXPECT_THROW(table.where(expression), std::invalid_argument) << expression;
    }
}

TEST(WhereExpressionTest, UnknownColumnIsNamedInTheError)
{
    const FITStable table = CreateSampleTable();

    try
    {
        table.where("NOPE > 0");
        FAIL() << "expected std::invalid_argument";
    }
    catch (const std::invalid_argument& e)
    {
        EXPECT_NE(std::string(e.what()).find("NOPE"), std::string::npos);
    }
}

TEST(WhereExpressionTest, VectorColumnsAreRejected)
{
    FITStable table;

    auto vec = std::make_shared<FITScolumn<FITSform::floatVector>>("VEC", tfloat, int64_t(2), int64_t(4), "", 1);
    vec->push_back(FITSform::floatVector{1.f, 2.f});
    table.InsertColumn(vec);

    EXPECT_THROW(table.where("VEC > 0"), std::invalid_argument);
}

TEST(WhereExpressionTest, WideIntegersKeepFullPrecision)
{
    FITStable table;

    // 2^53 and 2^53+1 are the same number once they go through a double.
    auto u64 = std::make_shared<FITScolumn<uint64_t>>("U64", tulonglong, "", 1);
    for (uint64_t value : std::vector<uint64_t>{9007199254740992ULL, 9007199254740993ULL, 18446744073709551615ULL})
    {
        u64->push_back(value);
    }
    table.InsertColumn(u64);

    auto i64 = std::make_shared<FITScolumn<int64_t>>("I64", tlonglong, "", 1);
    for (int64_t value : std::vector<int64_t>{-9007199254740993LL, 9007199254740992LL, 9007199254740993LL})
    {
        i64->push_back(value);
    }
    table.InsertColumn(i64);

    EXPECT_EQ(IndicesOf(table.where("U64 == 9007199254740993")), (std::vector<size_t>{1}));
    EXPECT_EQ(IndicesOf(table.where("U64 == 18446744073709551615")), (std::vector<size_t>{2}));
    EXPECT_EQ(IndicesOf(table.where("I64 == -9007199254740993")), (std::vector<size_t>{0}));
    EXPECT_EQ(IndicesOf(table.where("I64 > 9007199254740992")), (std::vector<size_t>{2}));

    EXPECT_THROW(table.where("U64 == 18446744073709551616"), std::invalid_argument);
    EXPECT_THROW(table.where("U64 > -1"), std::invalid_argument);
    EXPECT_THROW(table.where("I64 == 9223372036854775808"), std::invalid_argument);
}

TEST(WhereExpressionTest, UnsignedAndFloatColumns)
{
    FITStable table;

    auto u16 = std::make_shared<FITScolumn<uint16_t>>("U16", tushort, "", 1);
    for (uint16_t value : std::vector<uint16_t>{0, 40000, 65535})
    {
        u16->push_back(value);
    }
    table.InsertColumn(u16);

    auto u32 = std::make_shared<FITScolumn<uint32_t>>("U32", tuint, "", 1);
    for (uint32_t value : std::vector<uint32_t>{0u, 3000000000u, 4294967295u})
    {
        u32->push_back(value);
    }
    table.InsertColumn(u32);

    auto f32 = std::make_shared<FITScolumn<float>>("F32", tfloat, "", 1);
    for (float value : std::vector<float>{0.1f, 0.2f, 1.5f})
    {
        f32->push_back(value);
    }
    table.InsertColumn(f32);

    EXPECT_EQ(IndicesOf(table.where("U16 == 65535")), (std::vector<size_t>{2}));
    EXPECT_EQ(IndicesOf(table.where("U32 >= 3000000000")), (std::vector<size_t>{1, 2}));

    // Read as a double, 0.1 would differ from the stored 0.1f and match nothing.
    EXPECT_EQ(IndicesOf(table.where("F32 == 0.1")), (std::vector<size_t>{0}));

    EXPECT_THROW(table.where("U16 == 65536"), std::invalid_argument);
    EXPECT_THROW(table.where("U32 == 4294967296"), std::invalid_argument);
    EXPECT_THROW(table.where("F32 > 1e50"), std::invalid_argument);
}

TEST(WhereExpressionTest, RowsSelectTheMatchingValuesOfOtherColumns)
{
    FITStable table = CreateSampleTable();

    const RowSet rows = table.where("COL_STR =~ *eta && COL_INT > 3");

    std::vector<double> selected;
    rows.forEach([&](size_t row){ selected.push_back(table.column<double>("COL_DOUBLE").data()[row]); });

    // beta, zeta, eta and theta end in "eta"; of those only zeta holds more than 3.
    EXPECT_EQ(selected, (std::vector<double>{-2.3}));
}

namespace
{
    template<typename T>
    void InsertValues(FITStable& table, const std::string& name, const dtype& type, const std::vector<T>& values)
    {
        auto column = std::make_shared<FITScolumn<T>>(name, type, "", 1);
        for (const T& value : values)
        {
            column->push_back(value);
        }
        table.InsertColumn(column);
    }
}

TEST(WhereExpressionTest, HexadecimalOnEveryNumericColumn)
{
    FITStable table;

    InsertValues<int8_t>  (table, "I8",  tsbyte,     {-128, 127, 16});
    InsertValues<uint8_t> (table, "U8",  tbyte,      {0, 200, 255});
    InsertValues<int16_t> (table, "I16", tshort,     {-32768, 32767, 16});
    InsertValues<uint16_t>(table, "U16", tushort,    {0, 40000, 65535});
    InsertValues<int32_t> (table, "I32", tint,       {std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max(), 16});
    InsertValues<uint32_t>(table, "U32", tuint,      {0u, 3000000000u, 4294967295u});
    InsertValues<int64_t> (table, "I64", tlonglong,  {std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 9007199254740993LL});
    InsertValues<uint64_t>(table, "U64", tulonglong, {9007199254740992ULL, 9007199254740993ULL, std::numeric_limits<uint64_t>::max()});
    InsertValues<float>   (table, "F32", tfloat,     {0.5f, 1.5f, 16.f});
    InsertValues<double>  (table, "D64", tdouble,    {0.5, 42.0, 16.0});

    const std::vector<size_t> first{0}, second{1}, third{2};

    EXPECT_EQ(IndicesOf(table.where("I8 == -0x80")), first);
    EXPECT_EQ(IndicesOf(table.where("I8 == 0x7F")), second);
    EXPECT_EQ(IndicesOf(table.where("U8 == 0xC8")), second);
    EXPECT_EQ(IndicesOf(table.where("U8 == 0xff")), third);
    EXPECT_EQ(IndicesOf(table.where("I16 == -0x8000")), first);
    EXPECT_EQ(IndicesOf(table.where("I16 == 0x7FFF")), second);
    EXPECT_EQ(IndicesOf(table.where("U16 == 0xFFFF")), third);
    EXPECT_EQ(IndicesOf(table.where("I32 == -0x80000000")), first);
    EXPECT_EQ(IndicesOf(table.where("I32 == 0x7FFFFFFF")), second);
    EXPECT_EQ(IndicesOf(table.where("U32 == 0xFFFFFFFF")), third);
    EXPECT_EQ(IndicesOf(table.where("I64 == -0x8000000000000000")), first);
    EXPECT_EQ(IndicesOf(table.where("I64 == 0x7FFFFFFFFFFFFFFF")), second);
    EXPECT_EQ(IndicesOf(table.where("I64 == 0x20000000000001")), third); // 2^53+1: no double on the way
    EXPECT_EQ(IndicesOf(table.where("U64 == 0x20000000000001")), second);
    EXPECT_EQ(IndicesOf(table.where("U64 == 0XFFFFFFFFFFFFFFFF")), third);
    EXPECT_EQ(IndicesOf(table.where("F32 == 0x10")), third);
    EXPECT_EQ(IndicesOf(table.where("F32 == 0x1.8p0")), second);
    EXPECT_EQ(IndicesOf(table.where("D64 == 0x2A")), second);
    EXPECT_EQ(IndicesOf(table.where("D64 == 0x1p-1")), first);

    EXPECT_EQ(IndicesOf(table.where("I32==+0x10")), third);
    EXPECT_EQ(IndicesOf(table.where("(I8=0x10) && U8>=0xC8")), third);

    // A leading zero is still decimal, not octal: 010 is 10, which no row holds, not 8.
    EXPECT_TRUE(table.where("I32 == 010").empty());
    EXPECT_EQ(IndicesOf(table.where("I32 == 016")), third);

    EXPECT_THROW(table.where("I8 == 0x80"), std::invalid_argument);
    EXPECT_THROW(table.where("U8 == 0x100"), std::invalid_argument);
    EXPECT_THROW(table.where("U8 > -0x1"), std::invalid_argument);
    EXPECT_THROW(table.where("I16 == 0x8000"), std::invalid_argument);
    EXPECT_THROW(table.where("U16 == 0x10000"), std::invalid_argument);
    EXPECT_THROW(table.where("I32 == 0x80000000"), std::invalid_argument);
    EXPECT_THROW(table.where("U32 == 0x100000000"), std::invalid_argument);
    EXPECT_THROW(table.where("I64 == 0x8000000000000000"), std::invalid_argument);
    EXPECT_THROW(table.where("U64 == 0x10000000000000000"), std::invalid_argument);

    EXPECT_THROW(table.where("I32 == 0x"), std::invalid_argument);
    EXPECT_THROW(table.where("I32 == 0xZZ"), std::invalid_argument);
    EXPECT_THROW(table.where("I32 == 0x1.8"), std::invalid_argument);
    EXPECT_THROW(table.where("D64 == 0x"), std::invalid_argument);
    EXPECT_THROW(table.where("D64 == 0xZZ"), std::invalid_argument);
    EXPECT_THROW(table.where("F32 > 0x1p200"), std::invalid_argument);
}

TEST(WhereExpressionTest, DecimalPointIsADotWhateverTheLocale)
{
    const FITStable table = CreateSampleTable();
    const std::vector<size_t> expected{4, 6, 11};

    ASSERT_EQ(IndicesOf(table.where("COL_DOUBLE > 3.13")), expected);

    CommaDecimalLocale commaLocale;

    // Proves the locale is really in force, or the checks below would prove nothing.
    std::ostringstream printed;
    printed << 1.5;
    ASSERT_EQ(printed.str(), "1,5");

    EXPECT_EQ(IndicesOf(table.where("COL_DOUBLE > 3.13")), expected);
    EXPECT_EQ(IndicesOf(table.where("COL_DOUBLE == 0.25")), (std::vector<size_t>{2}));
    EXPECT_EQ(IndicesOf(table.where("COL_DOUBLE >= 4.2e1")), (std::vector<size_t>{11}));
    EXPECT_THROW(table.where("COL_DOUBLE > 3,13"), std::invalid_argument);
}

TEST(TableCsvTest, DecimalPointIsADotWhateverTheLocale)
{
    FITStable table;

    auto dbl = std::make_shared<FITScolumn<double>>("DBL", tdouble, "", 1);
    dbl->push_back(0.25);
    table.InsertColumn(dbl);

    auto flt = std::make_shared<FITScolumn<float>>("FLT", tfloat, "", 1);
    flt->push_back(0.5f);
    table.InsertColumn(flt);

    auto fltVec = std::make_shared<FITScolumn<FITSform::floatVector>>("FV", tfloat, int64_t(2), int64_t(4), "", 1);
    fltVec->push_back(FITSform::floatVector{1.5f, 2.5f});
    table.InsertColumn(fltVec);

    auto dblVec = std::make_shared<FITScolumn<FITSform::doubleVector>>("DV", tdouble, int64_t(2), int64_t(8), "", 1);
    dblVec->push_back(FITSform::doubleVector{3.5, 4.5});
    table.InsertColumn(dblVec);

    auto cplx = std::make_shared<FITScolumn<FITSform::complex>>("CX", tcplx, "", 1);
    cplx->push_back(FITSform::complex(5.5f, 6.5f));
    table.InsertColumn(cplx);

    auto dblCplx = std::make_shared<FITScolumn<FITSform::dblcomplex>>("DC", tdbcplx, "", 1);
    dblCplx->push_back(FITSform::dblcomplex(7.5, 8.5));
    table.InsertColumn(dblCplx);

    CommaDecimalLocale commaLocale;

    // Created under the comma locale, this stream would write 1,5 by itself.
    std::ostringstream csv;
    table.toCSV(csv, '\t');
    const std::string text = csv.str();

    for (const std::string& expected : {"0.25", "0.5", "{1.5;2.5}", "{3.5;4.5}", "(5.5,6.5)", "(7.5,8.5)"})
    {
        EXPECT_NE(text.find(expected), std::string::npos) << expected << " not in:\n" << text;
    }
}

TEST(WhereExpressionTest, TableReadBackFromFile)
{
#ifndef CI_CIWORKER
    const std::string fileName = "./build/testdata/test_fitstable_where.fits";
#else
    const std::string fileName = "./testdata/test_fitstable_where.fits";
#endif

    FITStable written;

    auto scsid = std::make_shared<FITScolumn<int16_t>>("SCSID", tshort, "", 1);
    for (int16_t value : std::vector<int16_t>{0, 1, 2, -1})
    {
        scsid->push_back(value);
    }
    written.InsertColumn(scsid);

    // Above 32767 a uint16 only survives the file through BZERO.
    auto counts = std::make_shared<FITScolumn<uint16_t>>("COUNTS", tushort, "", 1);
    for (uint16_t value : std::vector<uint16_t>{7, 40000, 65535, 0})
    {
        counts->push_back(value);
    }
    written.InsertColumn(counts);

    auto ra = std::make_shared<FITScolumn<double>>("RA", tdouble, "", 1);
    for (double value : std::vector<double>{10.5, 20.25, -3.0, 0.0})
    {
        ra->push_back(value);
    }
    written.InsertColumn(ra);

    auto flag = std::make_shared<FITScolumn<bool>>("FLAG", tlogical, "", 1);
    for (bool value : std::vector<bool>{true, false, true, false})
    {
        flag->push_back(value);
    }
    written.InsertColumn(flag);

    // Names shorter than the widest one come back from padded storage.
    auto name = std::make_shared<FITScolumn<std::string>>("OBJNAME", tstring, "", 1);
    for (const std::string& value : std::vector<std::string>{"WD 1", "WD12", "X", "lambda"})
    {
        name->push_back(value);
    }
    written.InsertColumn(name);

    ASSERT_NO_THROW(written.write(fileName, 0, true));

    fitsfile* rawFptr = nullptr;
    int status = 0;
    ASSERT_EQ(fits_open_file(&rawFptr, fileName.c_str(), READONLY, &status), 0);
    std::shared_ptr<fitsfile> fptr(rawFptr, [](fitsfile*){}); // manual close

    int hdutype = 0;
    ASSERT_EQ(fits_movabs_hdu(fptr.get(), 2, &hdutype, &status), 0);
    const FITStable table(fptr, 2);
    EXPECT_EQ(fits_close_file(fptr.get(), &status), 0);

    ASSERT_EQ(table.nrows(), 4u);

    EXPECT_EQ(table.where("").size(), 4u);
    EXPECT_EQ(IndicesOf(table.where("SCSID > 0")), (std::vector<size_t>{1, 2}));
    EXPECT_EQ(IndicesOf(table.where("COUNTS > 32767")), (std::vector<size_t>{1, 2}));
    EXPECT_EQ(IndicesOf(table.where("RA >= 20 && SCSID <= 1")), (std::vector<size_t>{1}));
    EXPECT_EQ(IndicesOf(table.where("FLAG == T")), (std::vector<size_t>{0, 2}));
    EXPECT_EQ(IndicesOf(table.where("OBJNAME == X")), (std::vector<size_t>{2}));
    EXPECT_EQ(IndicesOf(table.where("OBJNAME == 'WD 1'")), (std::vector<size_t>{0}));
    EXPECT_EQ(IndicesOf(table.where("OBJNAME =~ WD*")), (std::vector<size_t>{0, 1}));
    EXPECT_EQ(IndicesOf(table.where("OBJNAME !~ WD*")), (std::vector<size_t>{2, 3}));
    EXPECT_EQ(IndicesOf(table.where("(OBJNAME =~ WD* || SCSID < 0) && FLAG == F")), (std::vector<size_t>{1, 3}));

    EXPECT_THROW(table.where("SCSID > 70000"), std::invalid_argument);
    EXPECT_THROW(table.where("MISSING > 0"), std::invalid_argument);
}

namespace
{
    /// Four rows: real columns A and B, integer I, string S, 2-vectors V and W, a 3-vector T, complex Z.
    FITStable CreateComputeTable()
    {
        FITStable table;

        auto a = std::make_shared<FITScolumn<double>>("A", tdouble, "", 1);
        for (double value : {1.0, 2.0, 3.0, 4.0}) a->push_back(value);
        table.InsertColumn(a);

        auto b = std::make_shared<FITScolumn<float>>("B", tfloat, "", 2);
        for (float value : {0.5f, 0.0f, 2.0f, -1.0f}) b->push_back(value);
        table.InsertColumn(b);

        auto i = std::make_shared<FITScolumn<int32_t>>("I", tint, "", 3);
        for (int32_t value : {10, 20, 30, 40}) i->push_back(value);
        table.InsertColumn(i);

        auto s = std::make_shared<FITScolumn<std::string>>("S", tstring, "", 4);
        for (const char* value : {"a", "b", "c", "d"}) s->push_back(std::string(value));
        table.InsertColumn(s);

        auto v = std::make_shared<FITScolumn<FITSform::doubleVector>>("V", tdouble, int64_t(2), int64_t(8), "", 5);
        for (int row = 0; row < 4; ++row) v->push_back(FITSform::doubleVector{double(row), double(10 + row)});
        table.InsertColumn(v);

        auto w = std::make_shared<FITScolumn<FITSform::floatVector>>("W", tfloat, int64_t(2), int64_t(4), "", 6);
        for (int row = 0; row < 4; ++row) w->push_back(FITSform::floatVector{1.0f, 2.0f});
        table.InsertColumn(w);

        auto t = std::make_shared<FITScolumn<FITSform::doubleVector>>("T", tdouble, int64_t(3), int64_t(8), "", 7);
        for (int row = 0; row < 4; ++row) t->push_back(FITSform::doubleVector{1.0, 2.0, 3.0});
        table.InsertColumn(t);

        auto z = std::make_shared<FITScolumn<FITSform::complex>>("Z", tcplx, "", 8);
        for (int row = 0; row < 4; ++row) z->push_back(FITSform::complex{3.0f, 4.0f});
        table.InsertColumn(z);

        return table;
    }

    /// Computes "R = <expression>" on a fresh table and returns the scalar result.
    std::vector<double> Computed(const std::string& expression)
    {
        FITStable table = CreateComputeTable();
        return table.computeColumn("R = " + expression).values<double>();
    }

    /// The definition must be refused, and leave the table as it was.
    void ExpectRefused(const std::string& definition)
    {
        FITStable table = CreateComputeTable();
        const size_t columns = table.ncols();
        EXPECT_THROW(table.computeColumn(definition), std::invalid_argument) << definition;
        EXPECT_EQ(table.ncols(), columns) << definition;
    }
}

TEST(ComputeColumnTest, ArithmeticAndPrecedence)
{
    EXPECT_EQ(Computed("A - B"), (std::vector<double>{0.5, 2.0, 1.0, 5.0}));
    EXPECT_EQ(Computed("A + 2 * I"), (std::vector<double>{21.0, 42.0, 63.0, 84.0}));
    EXPECT_EQ(Computed("(A + 2) * I"), (std::vector<double>{30.0, 80.0, 150.0, 240.0}));
    EXPECT_EQ(Computed("I / 10 - A"), (std::vector<double>{0.0, 0.0, 0.0, 0.0}));
    EXPECT_EQ(Computed("A-1"), (std::vector<double>{0.0, 1.0, 2.0, 3.0}));
    EXPECT_EQ(Computed("'A' * 1.5e1"), (std::vector<double>{15.0, 30.0, 45.0, 60.0}));
}

TEST(ComputeColumnTest, PowerBindsTightestAndGroupsToTheRight)
{
    EXPECT_EQ(Computed("-A^2"), (std::vector<double>{-1.0, -4.0, -9.0, -16.0}));
    EXPECT_EQ(Computed("2 * A^2"), (std::vector<double>{2.0, 8.0, 18.0, 32.0}));
    EXPECT_EQ(Computed("2^3^2 + 0 * A"), (std::vector<double>{512.0, 512.0, 512.0, 512.0}));
    EXPECT_DOUBLE_EQ(Computed("A^-1")[3], 0.25);
    EXPECT_EQ(Computed("pow(A, 2)"), (std::vector<double>{1.0, 4.0, 9.0, 16.0}));
}

TEST(ComputeColumnTest, Functions)
{
    EXPECT_EQ(Computed("sqrt(A * A)"), (std::vector<double>{1.0, 2.0, 3.0, 4.0}));
    EXPECT_EQ(Computed("log10(A / A * 100)"), (std::vector<double>{2.0, 2.0, 2.0, 2.0}));
    EXPECT_EQ(Computed("abs(B)"), (std::vector<double>{0.5, 0.0, 2.0, 1.0}));
    EXPECT_DOUBLE_EQ(Computed("exp(log(A))")[2], 3.0);
}

TEST(ComputeColumnTest, ConstantFillsEveryRow)
{
    EXPECT_EQ(Computed("7"), (std::vector<double>{7.0, 7.0, 7.0, 7.0}));
    EXPECT_EQ(Computed("abs(-3) + 1"), (std::vector<double>{4.0, 4.0, 4.0, 4.0}));
}

TEST(ComputeColumnTest, DivisionByZeroIsNotAnError)
{
    const std::vector<double> ratio = Computed("A / B");
    EXPECT_DOUBLE_EQ(ratio[0], 2.0);
    EXPECT_TRUE(std::isinf(ratio[1]));

    EXPECT_TRUE(std::isnan(Computed("B / B")[1]));
}

TEST(ComputeColumnTest, VectorsWorkElementByElement)
{
    FITStable table = CreateComputeTable();

    const auto& scaled = table.computeColumn("SCALED = V / 10").values<FITSform::doubleVector>();
    ASSERT_EQ(scaled.size(), 4u);
    EXPECT_EQ(scaled[2], (FITSform::doubleVector{0.2, 1.2}));

    const auto& byScalar = table.computeColumn("BY_SCALAR = A * V").values<FITSform::doubleVector>();
    EXPECT_EQ(byScalar[1], (FITSform::doubleVector{2.0, 22.0}));

    const auto& sum = table.computeColumn("SUM = V + W").values<FITSform::doubleVector>();
    EXPECT_EQ(sum[3], (FITSform::doubleVector{4.0, 15.0}));

    EXPECT_EQ(table.getColumn("SCALED")->getNelem(), 2);
    EXPECT_EQ(table.getColumn("SCALED")->getType(), tdouble);

    ExpectRefused("BAD = V + T");
}

TEST(ComputeColumnTest, ComplexGoesThroughItsFunctions)
{
    EXPECT_EQ(Computed("real(Z)"), (std::vector<double>{3.0, 3.0, 3.0, 3.0}));
    EXPECT_EQ(Computed("img(Z)"), (std::vector<double>{4.0, 4.0, 4.0, 4.0}));
    EXPECT_EQ(Computed("imag(Z)"), (std::vector<double>{4.0, 4.0, 4.0, 4.0}));
    EXPECT_EQ(Computed("abs(Z)"), (std::vector<double>{5.0, 5.0, 5.0, 5.0}));
    EXPECT_DOUBLE_EQ(Computed("arg(Z)")[0], std::atan2(4.0, 3.0));
    EXPECT_EQ(Computed("abs(Z) * 2"), (std::vector<double>{10.0, 10.0, 10.0, 10.0}));

    EXPECT_EQ(Computed("real(A)"), (std::vector<double>{1.0, 2.0, 3.0, 4.0}));
    EXPECT_EQ(Computed("img(A)"), (std::vector<double>{0.0, 0.0, 0.0, 0.0}));
    EXPECT_DOUBLE_EQ(Computed("arg(B)")[3], std::atan2(0.0, -1.0));

    ExpectRefused("BAD = Z");
    ExpectRefused("BAD = Z * 2");
    ExpectRefused("BAD = -Z");
    ExpectRefused("BAD = sqrt(Z)");
}

TEST(ComputeColumnTest, EveryRowIsComputedAndFilteredAfterwards)
{
    FITStable table = CreateComputeTable();
    const RowSet before = table.where("A > 2");

    EXPECT_EQ(table.computeColumn("C = A * 2").size(), table.nrows());

    EXPECT_EQ(IndicesOf(table.where("C > 4")), IndicesOf(before));
    EXPECT_EQ(table.ncols(), CreateComputeTable().ncols() + 1);
    EXPECT_EQ(table.getColumn("C")->getName(), "C");

    // The column it is computed from may be another computed column.
    EXPECT_EQ(table.computeColumn("D = C + 1").values<double>(), (std::vector<double>{3.0, 5.0, 7.0, 9.0}));
}

TEST(ComputeColumnTest, QuotedNames)
{
    FITStable table = CreateComputeTable();
    EXPECT_EQ(table.computeColumn("'MAG-G' = A + 1").values<double>(), (std::vector<double>{2.0, 3.0, 4.0, 5.0}));
    EXPECT_EQ(table.computeColumn("OUT = 'MAG-G' * 2").values<double>(), (std::vector<double>{4.0, 6.0, 8.0, 10.0}));
}

TEST(ComputeColumnTest, BadDefinitionsLeaveTheTableUnchanged)
{
    for (const char* definition : {
             "", "   ", "A", "= A", "A + 1", "NEW", "NEW =", "NEW = A +", "NEW = (A", "NEW = A)", "NEW = A B",
             "NEW = 1.5.2", "NEW = A == 1", "NEW = 'A", "NEW = A % 2", "'' = A",
             "NEW = MISSING", "NEW = missing(A)", "NEW = sqrt()", "NEW = sqrt(A, B)", "NEW = pow(A)",
             "NEW = S", "NEW = S + 1"})
    {
        ExpectRefused(definition);
    }

    // A name the table already has: neither a column of the file nor a computed one is replaced.
    ExpectRefused("A = B");

    FITStable table = CreateComputeTable();
    table.computeColumn("C = A");
    EXPECT_THROW(table.computeColumn("C = B"), std::invalid_argument);
    EXPECT_EQ(table.getColumn("C")->values<double>()[0], 1.0);
}
