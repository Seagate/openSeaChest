#!/bin/sh

#This script is based on the original example found here: https://cirrus-ci.org/examples/#release-assets
#modifications made to upload only the single artificate necessary from each freeBSD build of
#openSeaChest. This script should be run after the tar.xz file is created by a successful build
#The other major change is that this script is posix shell compliant instead of requiring bash

if [ "$CIRRUS_RELEASE" = "" ] && [ "$CIRRUS_TAG" = "" ]; then
  echo "Not a release. No need to deploy!"
  exit 0
fi

if [ "$GITHUB_TOKEN" = "" ]; then
  echo "Please provide GitHub access token via encrypted GITHUB_TOKEN environment variable!"
  exit 1
fi

branchName=$(printf '%s' "$CIRRUS_BRANCH" | tr '/' '-')
if [ "$CIRRUS_TAG" != "" ]; then
  branchName=$(printf '%s' "$CIRRUS_TAG" | tr '/' '-')
elif [ "$CIRRUS_RELEASE" != "" ]; then
  branchName=$(printf '%s' "$CIRRUS_RELEASE" | tr '/' '-')
fi
file_content_type="application/octet-stream"
file_to_upload="$CIRRUS_WORKING_DIR/openSeaChest-$branchName-$(uname -s)-$(uname -r)-$(uname -m).tar.xz"

if [ -f "$file_to_upload" ]; then

    echo "Uploading $file_to_upload..."
    name=$(basename "$file_to_upload")
    if [ "$CIRRUS_TAG" != "" ]; then
      url_to_upload="https://uploads.github.com/repos/$CIRRUS_REPO_FULL_NAME/releases/tag/$CIRRUS_TAG/assets?name=$name"
    else 
      url_to_upload="https://uploads.github.com/repos/$CIRRUS_REPO_FULL_NAME/releases/tag/$CIRRUS_RELEASE/assets?name=$name"
    fi
    curl -X POST \
        --data-binary @"$file_to_upload" \
        --header "Authorization: token $GITHUB_TOKEN" \
        --header "Content-Type: $file_content_type" \
        "$url_to_upload"

else
    echo "tar file not found. Nothing to upload!"
fi