#include "calculator.h"

#include <cmath>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr std::string_view kAdd = "+";
constexpr std::string_view kSubtract = "-";
constexpr std::string_view kMultiply = "*";
constexpr std::string_view kDivide = "/";
constexpr std::string_view kPower = "**";

constexpr std::string_view kPrint = "=";
constexpr std::string_view kClear = "c";
constexpr std::string_view kSave = "s";
constexpr std::string_view kLoad = "l";
constexpr std::string_view kQuit = "q";

void PrintError(std::string_view message)
{
    std::cerr << "Error: " << message << '\n';
}

} // namespace

std::optional<Number> ReadNumber()
{
    Number value = 0;
    if (std::cin >> value) {
        return value;
    }

    PrintError("Numeric operand expected");
    return std::nullopt;
}

bool IsOperation(std::string_view token)
{
    return token == kAdd || token == kSubtract || token == kMultiply || token == kDivide ||
           token == kPower;
}

Number ApplyOperation(Number lhs, std::string_view operation, Number rhs)
{
    if (operation == kAdd) {
        return lhs + rhs;
    }

    if (operation == kSubtract) {
        return lhs - rhs;
    }

    if (operation == kMultiply) {
        return lhs * rhs;
    }

    if (operation == kDivide) {
        return lhs / rhs;
    }

    if (operation == kPower) {
        return std::pow(lhs, rhs);
    }

    throw std::invalid_argument("ApplyOperation: unknown operation");
}

bool RunCalculatorCycle()
{
    const std::optional<Number> firstNumber = ReadNumber();
    if (!firstNumber) {
        return false;
    }

    Number currentValue = *firstNumber;
    std::optional<Number> memory;

    std::string command;
    while (std::cin >> command) {
        if (IsOperation(command)) {
            const std::optional<Number> operand = ReadNumber();
            if (!operand) {
                return false;
            }

            if (command == kDivide && *operand == 0.0) {
                PrintError("Division by zero");
                return false;
            }

            currentValue = ApplyOperation(currentValue, command, *operand);
        }
        else if (command == kPrint) {
            std::cout << currentValue << '\n';
        }
        else if (command == kClear) {
            currentValue = 0;
        }
        else if (command == kSave) {
            memory = currentValue;
        }
        else if (command == kLoad) {
            if (!memory) {
                PrintError("Memory is empty");
                return false;
            }

            currentValue = *memory;
        }
        else if (command == kQuit) {
            return true;
        }
        else {
            PrintError("Unknown token " + command);
            return false;
        }
    }

    return false;
}
