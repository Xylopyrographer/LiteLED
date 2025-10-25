# Pull Request Creation Information

## PR Details

**Source Branch:** v3_dev  
**Target Branch:** develop  
**Repository:** Xylopyrographer/LiteLED  

**PR Title:** Add files for version 3

**PR Body:**
```
Add files for version 3 and project changes from v3_dev branch.
```

**PR Type:** Regular PR (not draft)  
**Merge Strategy:** Preserve commits as they exist on v3_dev  

## Additional Requirements
- No reviewers to be assigned
- No assignees to be assigned
- No labels to be assigned
- No milestones to be assigned

## Changes Summary

The v3_dev branch contains 1 commit ahead of develop:
- Commit: d50da99 - "Revise for version 3.0.0"

**File Changes:**
- 23 files changed
- 4,804 additions
- 1,269 deletions

**Key Changes:**
- Added new documentation: "Using LiteLED.md", "docs/LiteLED Architecture.md"
- Added new examples: periman_test, rainbow
- Added new source files: ll_encoder, ll_led_timings, ll_priority, ll_registry, ll_strip_core, ll_strip_pixels
- Updated existing files: README.md, LICENSE, library.properties, LiteLED.cpp, LiteLED.h, keywords.txt, llrgb.h, llrmt.h

## GitHub CLI Command

To create this PR using GitHub CLI (requires authentication):

```bash
gh pr create \
  --repo Xylopyrographer/LiteLED \
  --base develop \
  --head v3_dev \
  --title "Add files for version 3" \
  --body "Add files for version 3 and project changes from v3_dev branch."
```

## GitHub Web Interface

Alternatively, create the PR via GitHub web interface:
1. Navigate to: https://github.com/Xylopyrographer/LiteLED
2. Click "Pull requests" tab
3. Click "New pull request"
4. Set base: develop
5. Set compare: v3_dev
6. Click "Create pull request"
7. Title: "Add files for version 3"
8. Body: "Add files for version 3 and project changes from v3_dev branch."
9. Ensure "Create draft pull request" is NOT checked
10. Click "Create pull request"
