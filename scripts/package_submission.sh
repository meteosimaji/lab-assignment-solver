#!/bin/sh
set -eu

rm -f submit.zip
submission_files="assignment.c Makefile LICENSE README.md datasets rank_costs weights tests"
submission_files="$submission_files docs/ALGORITHM_PROOF.md docs/algorithm_paper.tex"
if [ -f docs/algorithm_paper.pdf ]; then
  submission_files="$submission_files docs/algorithm_paper.pdf"
fi
submission_files="$submission_files docs/SHOWCASE.md docs/OPTIMIZATION_NOTES.md"
submission_files="$submission_files docs/benchmark_results.tsv scripts"

zip -r submit.zip $submission_files \
  -x '*/__pycache__/*' \
  -x '*/.pytest_cache/*' \
  -x '*/.DS_Store' \
  -x '*/__MACOSX/*' \
  -x 'tmp/*' \
  -x 'assign_labs' \
  -x 'assign_labs.exe' \
  -x '*.dSYM/*'
