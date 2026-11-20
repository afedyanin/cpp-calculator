#pragma once

#include <optional>
#include <string_view>

using Number = double;

// Читает число из стандартного ввода.
// Возвращает std::nullopt, если ввод не является числом.
[[nodiscard]] std::optional<Number> ReadNumber();

// Проверяет, является ли токен бинарной операцией (+, -, *, /, **).
[[nodiscard]] bool IsOperation(std::string_view token);

// Применяет бинарную операцию к операндам.
// Предусловие: IsOperation(operation) == true,
// иначе выбрасывается std::invalid_argument.
[[nodiscard]] Number ApplyOperation(Number lhs, std::string_view operation, Number rhs);

// Запускает цикл калькулятора.
// Возвращает true, если работа завершилась штатно командой q,
// и false, если из-за ошибки или окончания ввода.
[[nodiscard]] bool RunCalculatorCycle();
