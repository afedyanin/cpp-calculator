#include "calculator.h"
#include "test_framework.h"

#include <optional>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <string_view>

namespace {

// Подменяет буферы std::cin, std::cout и std::cerr на строковые,
// чтобы проверять RunCalculatorCycle без настоящего консольного ввода-вывода.
class StreamRedirect {
  public:
    explicit StreamRedirect(const std::string& input)
        : input_(input), output_(), error_(), originalInput_(std::cin.rdbuf(input_.rdbuf())),
          originalOutput_(std::cout.rdbuf(output_.rdbuf())),
          originalError_(std::cerr.rdbuf(error_.rdbuf()))
    {
    }

    ~StreamRedirect()
    {
        std::cin.rdbuf(originalInput_);
        std::cout.rdbuf(originalOutput_);
        std::cerr.rdbuf(originalError_);
    }

    StreamRedirect(const StreamRedirect&) = delete;
    StreamRedirect& operator=(const StreamRedirect&) = delete;

    [[nodiscard]] std::string Output() const
    {
        return output_.str();
    }

    [[nodiscard]] std::string Errors() const
    {
        return error_.str();
    }

  private:
    std::istringstream input_;
    std::ostringstream output_;
    std::ostringstream error_;
    std::streambuf* originalInput_;
    std::streambuf* originalOutput_;
    std::streambuf* originalError_;
};

struct RunResult {
    bool completed = false;
    std::string output;
    std::string errors;
};

RunResult Run(std::string_view input)
{
    StreamRedirect redirect{std::string(input)};
    const bool completed = RunCalculatorCycle();

    RunResult result;
    result.completed = completed;
    result.output = redirect.Output();
    result.errors = redirect.Errors();
    return result;
}

} // namespace

// --- ReadNumber -------------------------------------------------------------

TEST(ReadNumberReadsInteger)
{
    StreamRedirect redirect{"42"};

    const std::optional<Number> value = ReadNumber();

    CHECK(value.has_value());
    CHECK_NEAR(*value, 42.0, 1e-9);
    CHECK_EQ(redirect.Errors(), "");
}

TEST(ReadNumberReadsFractionalNumber)
{
    StreamRedirect redirect{"2.5"};

    const std::optional<Number> value = ReadNumber();

    CHECK(value.has_value());
    CHECK_NEAR(*value, 2.5, 1e-9);
    CHECK_EQ(redirect.Errors(), "");
}

TEST(ReadNumberReadsNegativeNumber)
{
    StreamRedirect redirect{"-3.75"};

    const std::optional<Number> value = ReadNumber();

    CHECK(value.has_value());
    CHECK_NEAR(*value, -3.75, 1e-9);
    CHECK_EQ(redirect.Errors(), "");
}

TEST(ReadNumberReportsErrorOnText)
{
    StreamRedirect redirect{"abc"};

    const std::optional<Number> value = ReadNumber();

    CHECK(!value.has_value());
    CHECK_EQ(redirect.Errors(), "Error: Numeric operand expected\n");
}

TEST(ReadNumberReportsErrorOnEndOfInput)
{
    StreamRedirect redirect{""};

    const std::optional<Number> value = ReadNumber();

    CHECK(!value.has_value());
    CHECK_EQ(redirect.Errors(), "Error: Numeric operand expected\n");
}

// --- IsOperation ------------------------------------------------------------

TEST(IsOperationRecognizesAllOperations)
{
    CHECK(IsOperation("+"));
    CHECK(IsOperation("-"));
    CHECK(IsOperation("*"));
    CHECK(IsOperation("/"));
    CHECK(IsOperation("**"));
}

TEST(IsOperationRejectsCommandsAndOtherTokens)
{
    CHECK(!IsOperation(""));
    CHECK(!IsOperation("="));
    CHECK(!IsOperation("c"));
    CHECK(!IsOperation("s"));
    CHECK(!IsOperation("l"));
    CHECK(!IsOperation("q"));
    CHECK(!IsOperation("***"));
    CHECK(!IsOperation("2"));
}

// --- ApplyOperation ---------------------------------------------------------

TEST(ApplyOperationAdds)
{
    CHECK_NEAR(ApplyOperation(2, "+", 3), 5.0, 1e-12);
    CHECK_NEAR(ApplyOperation(-2, "+", 5), 3.0, 1e-12);
}

TEST(ApplyOperationSubtracts)
{
    CHECK_NEAR(ApplyOperation(2, "-", 3), -1.0, 1e-12);
}

TEST(ApplyOperationMultiplies)
{
    CHECK_NEAR(ApplyOperation(2.5, "*", 4), 10.0, 1e-12);
}

TEST(ApplyOperationDivides)
{
    CHECK_NEAR(ApplyOperation(7, "/", 2), 3.5, 1e-12);
    CHECK_NEAR(ApplyOperation(1, "/", 4), 0.25, 1e-12);
}

TEST(ApplyOperationRaisesToPower)
{
    CHECK_NEAR(ApplyOperation(2, "**", 10), 1024.0, 1e-12);
    CHECK_NEAR(ApplyOperation(2, "**", -1), 0.5, 1e-12);
}

TEST(ApplyOperationThrowsOnUnknownOperation)
{
    bool thrown = false;
    try {
        static_cast<void>(ApplyOperation(1, "%", 2));
    }
    catch (const std::invalid_argument&) {
        thrown = true;
    }

    CHECK(thrown);
}

// --- RunCalculatorCycle: арифметика и команды -------------------------------

TEST(RunCalculatorCycleAddsOperand)
{
    const RunResult result = Run("2 + 3 = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "5\n");
    CHECK_EQ(result.errors, "");
}

TEST(RunCalculatorCycleSubtractsOperand)
{
    const RunResult result = Run("10 - 4 = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "6\n");
}

TEST(RunCalculatorCycleMultipliesOperand)
{
    const RunResult result = Run("2 * 3 = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "6\n");
}

TEST(RunCalculatorCycleDividesOperand)
{
    const RunResult result = Run("7 / 2 = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "3.5\n");
}

TEST(RunCalculatorCycleRaisesToPower)
{
    const RunResult result = Run("2 ** 10 = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "1024\n");
}

TEST(RunCalculatorCycleAppliesOperationsLeftToRight)
{
    const RunResult result = Run("2 ** 3 ** 2 = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "64\n");
}

TEST(RunCalculatorCycleHandlesNegativeNumbers)
{
    const RunResult result = Run("-2 * 3 = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "-6\n");
}

TEST(RunCalculatorCycleHandlesNegativeExponent)
{
    const RunResult result = Run("2 ** -1 = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "0.5\n");
}

TEST(RunCalculatorCycleUsesDefaultPrecision)
{
    const RunResult result = Run("1 / 3 = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "0.333333\n");
}

TEST(RunCalculatorCycleClearsCurrentValue)
{
    const RunResult result = Run("7 c = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "0\n");
}

TEST(RunCalculatorCycleStoresAndLoadsMemory)
{
    const RunResult result = Run("5 s c l = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "5\n");
    CHECK_EQ(result.errors, "");
}

TEST(RunCalculatorCycleKeepsLastSavedMemory)
{
    const RunResult result = Run("7 s * 2 s c l = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "14\n");
}

TEST(RunCalculatorCyclePrintsValueSeveralTimes)
{
    const RunResult result = Run("1 + 1 = = q");

    CHECK(result.completed);
    CHECK_EQ(result.output, "2\n2\n");
}

TEST(RunCalculatorCycleAcceptsTokensOnSeparateLines)
{
    const RunResult result = Run("2\n+\n3\n=\nq\n");

    CHECK(result.completed);
    CHECK_EQ(result.output, "5\n");
}

TEST(RunCalculatorCycleIgnoresInputAfterQuit)
{
    const RunResult result = Run("0 q abc");

    CHECK(result.completed);
    CHECK_EQ(result.output, "");
    CHECK_EQ(result.errors, "");
}

// --- RunCalculatorCycle: ошибки ---------------------------------------------

TEST(RunCalculatorCycleReportsMissingFirstNumber)
{
    const RunResult result = Run("abc");

    CHECK(!result.completed);
    CHECK_EQ(result.output, "");
    CHECK_EQ(result.errors, "Error: Numeric operand expected\n");
}

TEST(RunCalculatorCycleReportsMissingOperand)
{
    const RunResult result = Run("2 + q");

    CHECK(!result.completed);
    CHECK_EQ(result.output, "");
    CHECK_EQ(result.errors, "Error: Numeric operand expected\n");
}

TEST(RunCalculatorCycleReportsEmptyMemory)
{
    const RunResult result = Run("5 l q");

    CHECK(!result.completed);
    CHECK_EQ(result.output, "");
    CHECK_EQ(result.errors, "Error: Memory is empty\n");
}

TEST(RunCalculatorCycleReportsUnknownToken)
{
    const RunResult result = Run("2 abc");

    CHECK(!result.completed);
    CHECK_EQ(result.output, "");
    CHECK_EQ(result.errors, "Error: Unknown token abc\n");
}

TEST(RunCalculatorCycleReportsDivisionByZero)
{
    const RunResult result = Run("2 / 0 = q");

    CHECK(!result.completed);
    CHECK_EQ(result.output, "");
    CHECK_EQ(result.errors, "Error: Division by zero\n");
}

TEST(RunCalculatorCycleStopsAtEndOfInput)
{
    const RunResult result = Run("2 + 3");

    CHECK(!result.completed);
    CHECK_EQ(result.output, "");
    CHECK_EQ(result.errors, "");
}

TEST(RunCalculatorCyclePrintsBeforeEndOfInput)
{
    const RunResult result = Run("2 + 3 =");

    CHECK(!result.completed);
    CHECK_EQ(result.output, "5\n");
    CHECK_EQ(result.errors, "");
}
