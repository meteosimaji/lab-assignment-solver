#!/bin/sh
set -eu

output_path="${1:-lab-assignment-solver-source.zip}"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "error: source archive must be created from a Git working tree" >&2
  exit 1
fi

if ! git diff --quiet || ! git diff --cached --quiet; then
  echo "error: tracked changes are not committed; refusing to archive stale HEAD" >&2
  echo "hint: commit first, or run git archive manually if you intentionally want HEAD" >&2
  exit 1
fi

git archive \
  --format=zip \
  --prefix=lab-assignment-solver/ \
  --output="$output_path" \
  HEAD

if command -v zipinfo >/dev/null 2>&1; then
  if zipinfo -1 "$output_path" | grep -E '(^|/)(\.git/|\.DS_Store|__MACOSX/|tmp/|submit\.zip|assignment\.plist|assign_labs(\.exe)?$|202.*\.pdf$)' >/dev/null; then
    echo "error: archive contains local/private/generated files" >&2
    exit 1
  fi
fi

echo "wrote $output_path"
