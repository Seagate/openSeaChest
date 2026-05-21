#!/usr/bin/env bash
# format-on-save hook
# Runs clang-format --style=file after every successful C/C++ file write.
#
# Environment variables:
#   SKIP_FORMAT_ON_SAVE - "true" to disable entirely (default: unset)
#   FORMAT_EXTENSIONS   - Colon-separated extensions to format (default: c:h:cpp:hpp:cc:cxx)

set -euo pipefail

if [[ "${SKIP_FORMAT_ON_SAVE:-}" == "true" ]]; then
    exit 0
fi

# Silently skip if clang-format is not in PATH
if ! command -v clang-format &>/dev/null; then
    exit 0
fi

PAYLOAD="$(cat)"

# ---------------------------------------------------------------------------
# Require jq for JSON parsing — skip silently if unavailable
# ---------------------------------------------------------------------------
if ! command -v jq &>/dev/null; then
    exit 0
fi

TOOL_NAME="$(printf '%s' "$PAYLOAD" | jq -r '.toolName // ""')"
RESULT_TYPE="$(printf '%s' "$PAYLOAD" | jq -r '.toolResult.resultType // ""')"

# Only act on successful file-write tools
[[ "$RESULT_TYPE" == "success" ]] || exit 0

case "$TOOL_NAME" in
    replace_string_in_file|create_file|multi_replace_string_in_file) ;;
    *) exit 0 ;;
esac

# ---------------------------------------------------------------------------
# Extension filter
# ---------------------------------------------------------------------------
EXTENSIONS_RAW="${FORMAT_EXTENSIONS:-c:h:cpp:hpp:cc:cxx}"
IFS=':' read -ra EXTENSIONS <<< "$EXTENSIONS_RAW"

should_format() {
    local file="$1"
    local ext="${file##*.}"
    local e
    for e in "${EXTENSIONS[@]}"; do
        [[ "${ext,,}" == "${e,,}" ]] && return 0
    done
    return 1
}

# ---------------------------------------------------------------------------
# Format a single file, resolving Windows paths in WSL/Git Bash if needed
# ---------------------------------------------------------------------------
format_file() {
    local file="$1"
    [[ -n "$file" ]] || return 0
    should_format "$file" || return 0

    # Convert Windows absolute path (C:\...) to POSIX for WSL/Git Bash
    if [[ "$file" =~ ^[A-Za-z]:[/\\] ]]; then
        if command -v wslpath &>/dev/null; then
            file="$(wslpath "$file")"
        else
            # Git Bash: replace drive letter and backslashes
            file="/${file:0:1}${file:2}"
            file="${file//\\//}"
        fi
    fi

    [[ -f "$file" ]] || return 0
    clang-format --style=file -i "$file" 2>/dev/null || true
}

# ---------------------------------------------------------------------------
# Extract file path(s) from toolArgs
# toolArgs is a JSON string (stringified), so use fromjson to parse it
# ---------------------------------------------------------------------------
TOOL_ARGS_RAW="$(printf '%s' "$PAYLOAD" | jq -r '.toolArgs // ""')"

case "$TOOL_NAME" in
    replace_string_in_file|create_file)
        FILE_PATH="$(printf '%s' "$TOOL_ARGS_RAW" \
            | jq -r 'if type == "string" then fromjson else . end | .filePath // ""')"
        format_file "$FILE_PATH"
        ;;
    multi_replace_string_in_file)
        # replacements is an array; each element has a filePath — deduplicate
        printf '%s' "$TOOL_ARGS_RAW" \
            | jq -r 'if type == "string" then fromjson else . end | .replacements[]?.filePath // ""' \
            | sort -u \
            | while IFS= read -r fp; do
                format_file "$fp"
              done
        ;;
esac

exit 0
