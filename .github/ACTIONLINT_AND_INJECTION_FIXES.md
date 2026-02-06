# Actionlint and Injection Vulnerability Fixes

## Overview

This document describes the actionlint errors and injection vulnerabilities that were identified and fixed in the GitHub Actions workflows.

## Issues Identified

### 1. Actionlint Errors: Missing workflow_call Inputs

Three workflows were referencing `inputs.repo` and `inputs.pr-base-sha` without defining them in their `workflow_call` trigger inputs:

- `.github/workflows/clang-format-check.yaml`
- `.github/workflows/clang-tidy-check.yaml`
- `.github/workflows/coverage.yaml`

**Error Messages:**
```
property "repo" is not defined in object type {ref: string}
property "pr-base-sha" is not defined in object type {ref: string}
```

**Root Cause:** These workflows had logic to handle `workflow_call` events (e.g., `github.event_name == 'workflow_call' && inputs.repo`) but didn't declare the `workflow_call` trigger with the required inputs.

### 2. Injection Vulnerability in cmake-build.yaml

Line 211 in `cmake-build.yaml` had a shell injection vulnerability:

```yaml
run: echo "name=$(echo '${{ needs.pre-check.outputs.repo }}' | sed 's:.*/::')" >> "$GITHUB_OUTPUT"
```

**Security Risk:** The `${{ needs.pre-check.outputs.repo }}` expression is directly interpolated into the shell command. If an attacker could control the value of `needs.pre-check.outputs.repo`, they could inject arbitrary shell commands.

**Example Attack:** If `repo` contained `'; malicious-command; echo '`, it would be executed.

## Fixes Applied

### Fix 1: Add workflow_call Inputs

Added `workflow_call` trigger with required inputs to all three workflows:

```yaml
workflow_call:
  inputs:
    ref:
      description: "The branch, ref, or SHA to checkout"
      required: false
      type: string
    repo:
      description: "The repository to checkout from"
      required: false
      type: string
    pr-base-sha:
      description: "Base SHA of the PR for relevance check"
      required: false
      type: string
    pr-head-sha:
      description: "Head SHA of the PR for relevance check"
      required: false
      type: string
```

For `coverage.yaml`, also added the coverage-specific inputs:
```yaml
    phlex-coverage-compiler:
      description: 'Compiler to use for coverage build (gcc or clang)'
      required: false
      type: string
    phlex-enable-form:
      description: 'Enable FORM integration (set to OFF to exclude FORM sources)'
      required: false
      type: string
```

### Fix 2: Mitigate Injection Vulnerability

Changed the shell command in `cmake-build.yaml` to use an environment variable:

**Before:**
```yaml
- name: Extract repository name
  id: repo_name
  run: echo "name=$(echo '${{ needs.pre-check.outputs.repo }}' | sed 's:.*/::')" >> "$GITHUB_OUTPUT"
```

**After:**
```yaml
- name: Extract repository name
  id: repo_name
  env:
    REPO_FULL_NAME: ${{ needs.pre-check.outputs.repo }}
  run: echo "name=${REPO_FULL_NAME##*/}" >> "$GITHUB_OUTPUT"
```

**Security Improvement:**
1. The GitHub expression `${{ needs.pre-check.outputs.repo }}` is now assigned to an environment variable `REPO_FULL_NAME`
2. Environment variables are not subject to shell injection - they are passed as values, not code
3. Bonus: Replaced `sed` with bash parameter expansion (`${REPO_FULL_NAME##*/}`) which is more efficient and addresses a shellcheck style warning

## Best Practices for Preventing Injection

When using GitHub Actions expressions in shell commands:

1. **Use environment variables:** Always pass untrusted data through environment variables
   ```yaml
   env:
     MY_VAR: ${{ github.event.issue.title }}
   run: echo "$MY_VAR"
   ```

2. **Never interpolate directly:** Avoid this pattern:
   ```yaml
   run: echo "${{ github.event.issue.title }}"  # DANGEROUS
   ```

3. **Validate inputs:** Use GitHub Actions input validation when possible

4. **Use actions instead of scripts:** When feasible, use composite actions or JavaScript actions instead of shell scripts

## Verification

All fixes were verified using:

1. **actionlint:** Ran actionlint on all workflows - 0 errors reported
2. **YAML validation:** Verified all workflow files have valid YAML syntax
3. **Shellcheck:** The improved shell script addresses shellcheck style warnings

## References

- [GitHub Actions Security Hardening](https://docs.github.com/en/actions/security-guides/security-hardening-for-github-actions)
- [actionlint Documentation](https://github.com/rhysd/actionlint)
- [Preventing Script Injection Attacks](https://docs.github.com/en/actions/security-guides/security-hardening-for-github-actions#understanding-the-risk-of-script-injections)
