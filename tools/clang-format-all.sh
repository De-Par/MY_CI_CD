#!/usr/bin/env bash
# Запускает clang-format для всех *.cpp/*.hpp в src/include/app/tests.
# Опционально для локального использования перед push.

set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1; then
  echo "clang-format не найден. Установите его из пакета clang-format или через пакетный менеджер." >&2
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

#echo "Проверяю файлы:"
#echo "${FILES}"

tmpdir=$(mktemp -d)
for f in ${FILES}; do
  mkdir -p "${tmpdir}/$(dirname "$f")"
  cp "$f" "${tmpdir}/$f"
done

clang-format -i ${FILES}

changed=""
for f in ${FILES}; do
  if ! cmp -s "$f" "${tmpdir}/$f"; then
    changed="${changed}
$f"
  fi
done
rm -rf "$tmpdir"

echo "Форматирование..."

if [ -z "$changed" ]; then
  echo "Изменений нет, всё в порядке"
else
  echo "Отформатированы файлы:"
  while IFS= read -r line; do
    [ -n "$line" ] && echo "🟡 $line"
  done <<< "$changed"
fi

echo "✅ Готово"
