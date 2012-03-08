#!/bin/sh

ICON_FILE="${1}"
SPLASH_FILE="${2}"
BUNDLE_PATH="${3}"

# Create various sized icons.
sips -s format png "$ICON_FILE" --resampleHeightWidth 57 57 --out "$BUNDLE_PATH/Icon.png"
sips -s format png "$ICON_FILE" --resampleHeightWidth 114 114 --out "$BUNDLE_PATH/Icon@2x.png"
sips -s format png "$ICON_FILE" --resampleHeightWidth 72 72 --out "$BUNDLE_PATH/Icon-72.png"
sips -s format png "$ICON_FILE" --resampleHeightWidth 29 29 --out "$BUNDLE_PATH/Icon-Small.png"
sips -s format png "$ICON_FILE" --resampleHeightWidth 50 50 --out "$BUNDLE_PATH/Icon-Small-50.png"
sips -s format png "$ICON_FILE" --resampleHeightWidth 58 58 --out "$BUNDLE_PATH/Icon-Small@2x.png"

# Create splash screen.
sips -s format png "$SPLASH_FILE" --resampleHeightWidth 1024 768 --out "$BUNDLE_PATH/Default.png"
