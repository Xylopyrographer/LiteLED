#!/bin/bash

# Script to create PR from v3_dev to develop
# This script requires GitHub CLI (gh) to be installed and authenticated

set -e

REPO="Xylopyrographer/LiteLED"
BASE_BRANCH="develop"
HEAD_BRANCH="v3_dev"
PR_TITLE="Add files for version 3"
PR_BODY="Add files for version 3 and project changes from v3_dev branch."

echo "Creating pull request..."
echo "Repository: $REPO"
echo "Base branch: $BASE_BRANCH"
echo "Head branch: $HEAD_BRANCH"
echo "Title: $PR_TITLE"
echo ""

# Check if gh is installed
if ! command -v gh &> /dev/null; then
    echo "Error: GitHub CLI (gh) is not installed"
    echo "Please install it from: https://cli.github.com/"
    exit 1
fi

# Check if authenticated
if ! gh auth status &> /dev/null; then
    echo "Error: Not authenticated with GitHub CLI"
    echo "Please run: gh auth login"
    exit 1
fi

# Create the PR
PR_URL=$(gh pr create \
    --repo "$REPO" \
    --base "$BASE_BRANCH" \
    --head "$HEAD_BRANCH" \
    --title "$PR_TITLE" \
    --body "$PR_BODY" \
    2>&1)

if [ $? -eq 0 ]; then
    echo "✓ Pull request created successfully!"
    echo ""
    echo "PR URL: $PR_URL"
    echo ""
    
    # Get PR metadata
    PR_NUMBER=$(echo "$PR_URL" | grep -oP '/pull/\K\d+')
    
    if [ -n "$PR_NUMBER" ]; then
        echo "Fetching PR metadata..."
        gh pr view "$PR_NUMBER" --repo "$REPO" --json number,title,state,url,createdAt,author
    fi
else
    echo "✗ Failed to create pull request"
    echo "$PR_URL"
    exit 1
fi
