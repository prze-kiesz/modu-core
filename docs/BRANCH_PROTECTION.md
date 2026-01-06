# Branch Protection Configuration

This document describes how to configure branch protection rules for the `main` branch to enforce the pull request workflow.

## Prerequisites

- Repository admin access
- GitHub repository settings access

## Configuration Steps

### 1. Navigate to Branch Protection Settings

1. Go to your GitHub repository: https://github.com/prze-kiesz/modu-core
2. Click **Settings** (top menu)
3. Click **Branches** (left sidebar, under "Code and automation")
4. Click **Add branch protection rule** (or edit existing rule for `main`)

### 2. Branch Name Pattern

```
main
```

### 3. Required Settings

#### Pull Request Requirements

- ✅ **Require a pull request before merging**
  - ✅ **Require approvals**: 1
  - ⚪ **Include administrators**: UNCHECKED
    - Administrators can merge without approval
    - Non-admin contributors require 1 approval
  - ✅ **Dismiss stale pull request approvals when new commits are pushed**
  - ✅ **Require review from Code Owners** (optional, if CODEOWNERS file exists)
  - ⚠️ **Require approval of the most recent reviewable push**

#### Status Check Requirements

- ✅ **Require status checks to pass before merging**
  - ✅ **Require branches to be up to date before merging**
  - Search and add these status checks:
    - `build-test-debug` (Build and Test - Debug)
    - `build-test-release` (Build and Test - Release)
    - `static-analysis-clang-tidy` (Static Analysis - clang-tidy)
    - `static-analysis-cppcheck` (Static Analysis - cppcheck)
    - `static-analysis-clang-format` (Static Analysis - clang-format)

> **Note**: Status check names must match the job names in your GitHub Actions workflows. After the first PR runs these workflows, they will appear in the status checks dropdown.

#### Conversation Requirements

- ✅ **Require conversation resolution before merging**
  - Ensures all review comments are addressed

#### Commit Signing

- ⚠️ **Require signed commits** (optional but recommended for security)

#### History Requirements

- ✅ **Require linear history**
  - Enforces squash merging or rebasing (no merge commits)
  - Keeps history clean and easy to follow

#### Administrative Enforcement

- ⚪ **Do not allow bypassing the above settings**: **UNCHECKED**
  - Allows administrators to merge without approval
  - Non-admin contributors still require approvals
  - All users (including admins) must pass CI/CD checks

- ❌ **Allow force pushes**: **DISABLED** (unchecked)
  - Prevents force pushes to main branch

- ❌ **Allow deletions**: **DISABLED** (unchecked)
  - Prevents accidental branch deletion

### 4. Additional Recommendations

#### Ruleset (Alternative to Branch Protection)

GitHub now offers **Rulesets** as a more flexible alternative:

1. Go to **Settings** → **Rules** → **Rulesets**
2. Create new ruleset for `main` branch
3. Configure same protections as above
4. Benefit: Can apply rules to multiple branches with patterns

#### Required Workflows

- Consider adding required workflows that must pass
- Can enforce organization-wide security scans

#### Auto-merge Settings

- ⚠️ **Allow auto-merge** (optional)
  - Enables auto-merge when all requirements are met
  - Useful for automated dependency updates (Dependabot)

### 5. Verify Configuration

After setting up, test the protection:

```bash
# Try to push directly to main (should fail)
git checkout main
echo "test" >> test.txt
git add test.txt
git commit -m "test direct push"
git push origin main

# Expected error:
# remote: error: GH006: Protected branch update failed for refs/heads/main.
```

## Recommended Configuration Summary

For a repository with one administrator and potential external contributors:

```
✅ Require pull request before merging
   ✅ Require approvals: 1
   ⚪ Include administrators: UNCHECKED (allows admin to self-merge)
✅ Require status checks to pass before merging
   ✅ Require branches to be up to date
   ✅ build-test-debug
   ✅ build-test-release
   ✅ static-analysis-clang-tidy
   ✅ static-analysis-cppcheck
   ✅ static-analysis-clang-format
✅ Require conversation resolution before merging
✅ Require linear history (squash/rebase only)
⚪ Do not allow bypassing: UNCHECKED (admin bypass enabled)
❌ Allow force pushes: OFF
❌ Allow deletions: OFF
```

**Benefits:**
- Administrator can merge quickly after CI/CD passes
- External contributors require code review
- All changes must pass automated checks
- Clean linear history maintained

## Current Workflow Configuration

The repository has three GitHub Actions workflows that will run on pull requests:

### Build and Test Workflow

**File**: `.github/workflows/build-and-test.yml`

**Triggers**: 
- Pull requests to `main`
- Push to `main` (after merge)

**Jobs**:
- `build-and-test` with matrix: Debug, Release
- Status check names: `build-test-debug`, `build-test-release`

### Static Analysis Workflow

**File**: `.github/workflows/static-analysis.yml`

**Triggers**:
- Pull requests to `main`
- Push to `main` (after merge)

**Jobs**:
- `clang-tidy` - Status: `static-analysis-clang-tidy`
- `cppcheck` - Status: `static-analysis-cppcheck`
- `clang-format-check` - Status: `static-analysis-clang-format`

### Docker Build Workflow

**File**: `.github/workflows/docker-build.yml`

**Triggers**:
- Changes to `.devcontainer/Dockerfile`
- Pull requests (builds but doesn't publish)
- Releases and tags (publishes with versioning)

## Updating Workflows for PR Comments

Consider adding workflow steps to comment on PRs with:
- Test results summary
- Code coverage reports
- Static analysis findings
- Build artifact links

Example workflow addition:

```yaml
- name: Comment PR with results
  uses: actions/github-script@v7
  if: github.event_name == 'pull_request'
  with:
    script: |
      github.rest.issues.createComment({
        issue_number: context.issue.number,
        owner: context.repo.owner,
        repo: context.repo.repo,
        body: 'Build completed! ✅\n\nAll tests passed.'
      })
```

## Troubleshooting

### Status Checks Not Appearing

If status checks don't appear in the branch protection settings:

1. Create a test PR
2. Let workflows run
3. Once workflows complete, go back to branch protection settings
4. Status checks should now be available in the dropdown

### Administrator Bypass

If you're an admin and need to bypass temporarily:

1. Uncheck "Include administrators" in branch protection
2. Make your change
3. Re-enable "Include administrators"

**Note**: This is discouraged in production environments.

### Stale Branches

Configure automatic stale branch deletion:

1. **Settings** → **General**
2. Scroll to **Pull Requests**
3. ✅ **Automatically delete head branches**

## Maintenance

- Review branch protection rules quarterly
- Update status check requirements when workflows change
- Monitor bypass attempts in audit log
- Adjust approval requirements based on team size

## References

- [GitHub Branch Protection Documentation](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches)
- [GitHub Rulesets Documentation](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/about-rulesets)
- [Required Status Checks](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches#require-status-checks-before-merging)

---

**Last Updated**: January 6, 2026
