#!/usr/bin/env bash
# Запускает clang-format для всех *.cpp/*.hpp в src/include/app/tests.
# Использует системный clang-format из PATH.

set -euo pipefail

CLANG_FORMAT_BIN="clang-format"

if ! command -v "$CLANG_FORMAT_BIN" >/dev/null 2>&1; then
  echo "clang-format не найден. Установите его через пакетный менеджер и убедитесь, что он в PATH." >&2
  exit 1
fi

SEARCH_DIRS=(
    "src" 
    "include" 
    "app" 
    "tests"
)

FILES=$(find "${SEARCH_DIRS[@]}" -name '*.cpp' -o -name '*.hpp')

if [ -z "${FILES}" ]; then
  echo "Нет файлов для форматирования"
  exit 0
fi

tmpdir=$(mktemp -d)
for f in ${FILES}; do
  mkdir -p "${tmpdir}/$(dirname "$f")"
  cp "$f" "${tmpdir}/$f"
done

"$CLANG_FORMAT_BIN" -i ${FILES}

changed=""
for f in ${FILES}; do
  if ! cmp -s "$f" "${tmpdir}/$f"; then
    changed="${changed} $f"
  fi
done
rm -rf "$tmpdir"

echo "Форматирование..."

if [ -z "$changed" ]; then
  echo "Изменений нет, всё в порядке"
else
  echo "Отформатированы файлы:"
  echo "----------------------"
  while IFS= read -r line; do
    [ -n "$line" ] && echo "🟡 $line"
  done <<< "$changed"
fi

echo "----------------------"
echo "✅ Готово"
