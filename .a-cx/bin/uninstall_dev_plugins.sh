#!/bin/bash -u
#
# Copyright 2025 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Removes any installed Eclipsa Audio Renderer / Audio Element plugin
# bundles (VST3, AU, AAX) from every location a DAW scans on this Mac, so
# that a fresh local build is guaranteed to be picked up instead of a
# stale installed copy.
#
# Usage:
#   ./uninstall_dev_plugins.sh            # prompts once, then removes
#   ./uninstall_dev_plugins.sh --yes      # skip the confirmation prompt
#   ./uninstall_dev_plugins.sh --dry-run  # list what would be removed only

DRY_RUN=false
ASSUME_YES=false

for arg in "$@"; do
    case "$arg" in
        --dry-run) DRY_RUN=true ;;
        --yes|-y) ASSUME_YES=true ;;
        *)
            echo "Unknown argument: $arg" >&2
            echo "Usage: $0 [--dry-run] [--yes]" >&2
            exit 1
            ;;
    esac
done

PLUGIN_BASE_NAMES=(
    "Eclipsa Audio Renderer"
    "Eclipsa Audio Element Plugin"
)

# Directories a DAW scans for each format, system-wide and per-user.
VST3_DIRS=(
    "/Library/Audio/Plug-Ins/VST3"
    "$HOME/Library/Audio/Plug-Ins/VST3"
)
AU_DIRS=(
    "/Library/Audio/Plug-Ins/Components"
    "$HOME/Library/Audio/Plug-Ins/Components"
)
# AAX (Pro Tools) plugins are only ever installed system-wide.
AAX_DIRS=(
    "/Library/Application Support/Avid/Audio/Plug-Ins"
)

# Build the full list of candidate bundle paths to remove.
TARGETS=()
for base in "${PLUGIN_BASE_NAMES[@]}"; do
    for dir in "${VST3_DIRS[@]}"; do
        TARGETS+=("$dir/${base}.vst3")
    done
    for dir in "${AU_DIRS[@]}"; do
        TARGETS+=("$dir/${base}.component")
        # Some builds (ECLIPSA_LOGIC_PRO_BUILD) name the AU bundle differently.
        TARGETS+=("$dir/${base} for Logic Pro.component")
    done
    for dir in "${AAX_DIRS[@]}"; do
        TARGETS+=("$dir/${base}.aaxplugin")
    done
done

# Filter down to paths that actually exist.
FOUND=()
for path in "${TARGETS[@]}"; do
    if [ -e "$path" ]; then
        FOUND+=("$path")
    fi
done

if [ "${#FOUND[@]}" -eq 0 ]; then
    echo "No installed Eclipsa plugins found. Nothing to do."
    exit 0
fi

echo "Found ${#FOUND[@]} installed Eclipsa plugin bundle(s):"
for path in "${FOUND[@]}"; do
    echo "  $path"
done

if [ "$DRY_RUN" = true ]; then
    echo "Dry run: nothing removed."
    exit 0
fi

if [ "$ASSUME_YES" != true ]; then
    read -r -p "Remove all of the above? [y/N] " reply
    case "$reply" in
        [yY]|[yY][eE][sS]) ;;
        *)
            echo "Aborted. Nothing removed."
            exit 1
            ;;
    esac
fi

for path in "${FOUND[@]}"; do
    dir="$(dirname "$path")"
    if [ -w "$dir" ]; then
        rm -rf "$path"
    else
        echo "Elevated permissions required to remove: $path"
        sudo rm -rf "$path"
    fi
    if [ -e "$path" ]; then
        echo "WARNING: Failed to remove $path"
    else
        echo "Removed: $path"
    fi
done

echo "Done. Rebuild and reinstall to pick up fresh plugin binaries."
