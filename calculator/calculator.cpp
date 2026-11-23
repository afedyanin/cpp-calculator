#include "calculator.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr std::string_view kAdd = "+";
constexpr std::string_view kSubtract = "-";
constexpr std::string_view kMultiply = "*";
constexpr std::string_view kDivide = "/";
constexpr std::string_view kPower = "**";

constexpr std::string_view kPrint = "=";
constexpr std::string_view kClear = "c";
constexpr std::string_view kAssign = ":";
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

Number ApplyOperation(Number left_operand, std::string_view operation, Number right_operand)
{
    if (operation == kAdd) {
        return left_operand + right_operand;
    }

    if (operation == kSubtract) {
        return left_operand - right_operand;
    }

    if (operation == kMultiply) {
        return left_operand * right_operand;
    }

    if (operation == kDivide) {
        return left_operand / right_operand;
    }

    if (operation == kPower) {
        return std::pow(left_operand, right_operand);
    }

    throw std::invalid_argument("ApplyOperation: unknown operation");
}

bool RunCalculatorCycle()
{
    const std::optional<Number> first_number = ReadNumber();
    if (!first_number) {
        return false;
    }

    Number current_value = *first_number;
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

            current_value = ApplyOperation(current_value, command, *operand);
        }
        else if (command == kPrint) {
            std::cout << current_value << '\n';
        }
        else if (command == kClear) {
            current_value = 0;
        }
        else if (command == kAssign) {
            const std::optional<Number> new_value = ReadNumber();
            if (!new_value) {
                return false;
            }

            current_value = *new_value;
        }
        else if (command == kSave) {
            memory = current_value;
        }
        else if (command == kLoad) {
            if (!memory) {
                PrintError("Memory is empty");
                return false;
            }

            current_value = *memory;
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
