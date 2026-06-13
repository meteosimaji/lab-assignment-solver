#!/bin/sh
set -eu

rm -f submit.zip
zip -r submit.zip \
  assignment.c \
  Makefile \
  LICENSE \
  README.md \
  datasets \
  rank_costs \
  weights \
  tests \
  docs/algorithm_paper.tex \
  docs/algorithm_paper.pdf \
  docs/benchmark_results.tsv \
  scripts \
  -x '*/__pycache__/*' \
  -x '*/.pytest_cache/*' \
  -x '*/.DS_Store' \
  -x '*/__MACOSX/*' \
  -x 'tmp/*' \
  -x 'assign_labs' \
  -x 'assign_labs.exe' \
  -x '*.dSYM/*'
