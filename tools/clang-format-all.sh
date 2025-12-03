#!/usr/bin/env bash
# Запускает clang-format для всех *.cpp/*.hpp в src/include/app/tests.
# Опционально для локального использования перед push.

set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1; then
  echo "clang-format не найден. Установите его из пакета clang-format или через пакетный менеджер." >&2
  exit 1
fi

FILES=$(find src include app tests -name '*.cpp' -o -name '*.hpp')

if [ -z "${FILES}" ]; then
  echo "Нет файлов для форматирования."
  exit 0
fi

echo "Formatting files:"
echo "${FILES}"
clang-format -i ${FILES}

echo "Готово. Проверьте git diff перед коммитом."
