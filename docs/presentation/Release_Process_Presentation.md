---
marp: true
theme: default
paginate: true
header: "modu-core — Release Process"
footer: "Team Onboarding"
style: |
  section {
    font-size: 22px;
  }
  h1 {
    color: #2563eb;
    font-size: 32px;
  }
  h2 {
    color: #1e40af;
  }
  table {
    font-size: 18px;
  }
  code {
    font-size: 16px;
  }
  pre {
    font-size: 15px;
    line-height: 1.3;
  }
---

# Release Process
## modu-core

How we version, branch, and release software.

---

# Agenda

1. **Version scheme** — how versions are numbered
2. **Branching model** — where code lives
3. **CI/CD workflows** — what's automated
4. **How to: Feature release** — step by step
5. **How to: Hotfix release** — step by step
6. **Rules & branch protection**
7. **Q&A**

---

# Version Scheme

```
modu-core-v MAJOR . MINOR . PATCH  [-dev.N]
             │       │       │        │
             │       │       │        └── auto: every merge to main
             │       │       └── auto: hotfix from release branch
             │       └── auto: feature release from main
             └── manual: edit VERSION file (breaking change / milestone)
```

### Examples

| Tag | Meaning |
|-----|---------|
| `modu-core-v1.2.0` | Feature release (MINOR bump) |
| `modu-core-v1.2.1` | Hotfix release (PATCH bump) |
| `modu-core-v1.2.0-dev.3` | 3rd merge to main after v1.2.0 |

---

# VERSION File — Single Source of Truth

The `VERSION` file in the repo root:

```bash
MAJOR=1
```

- **CMake** reads it via `-DMODU_CORE_VERSION=modu-core-vX.Y.Z` (CI passes it)
- **Yocto** recipes can `source VERSION && echo $MAJOR`
- **Local builds** fall back to `modu-core-v0.0.0-local.0`

> **Rule:** MAJOR is the only thing a developer ever sets manually.
> Everything else is auto-incremented by CI.

---

# Branching Model

```
     main                        release/v1.2.x
       │
       ● merge PR  ← v1.0.0-dev.1
       ● merge PR  ← v1.0.0-dev.2
       ● Release   ← v1.1.0
       ● merge PR  ← v1.1.0-dev.1
       ● Release   ← v1.2.0
       │                    ┌── create-release-branch
       │                    ● hotfix PR
       │                    ● Release  ← v1.2.1
       │                    ● hotfix PR
       │                    ● Release  ← v1.2.2
       ● merge PR  ← v1.2.0-dev.1
       ▼
```

---

# Branching Rules

| Branch | Purpose | Protection |
|--------|---------|------------|
| `main` | All feature development | PR required, code owner review |
| `release/vX.Y.x` | Hotfix work for a released version | PR required, code owner review, no deletion |
| `feature/*`, `fix/*` | Developer work branches | Auto-deleted after merge |

### Key principles

- **All work** lands on `main` via pull requests
- **Release branches** are created only when a hotfix is needed
- **Tags are never created manually** — always through GitHub Actions
- Feature branches are **automatically deleted** after merge

---

# CI/CD Workflows — Overview

| Workflow | Trigger | What it does |
|----------|---------|-------------|
| **build-and-test** | Push / PR | Build + unit tests + acceptance tests |
| **static-analysis** | Push / PR (C++ files) | clang-tidy, cppcheck, clang-format |
| **docker-build** | `.devcontainer/` changes | Build & push devcontainer image |
| **dev-tag** | Every push to `main` | Auto-tag: `vX.Y.Z-dev.N` |
| **release** | Manual (select branch) | Build → .deb → tag → GitHub Release |
| **create-release-branch** | Manual | Create `release/vX.Y.x` from a tag |

---

# Automatic: What Happens on Every PR

```
Developer pushes PR to main
        │
        ▼
 ┌──────────────┐    ┌────────────────┐
 │ build-and-test│    │ static-analysis│
 │ • Debug build │    │ • clang-tidy   │
 │ • Release     │    │ • cppcheck     │
 │ • Unit tests  │    │ • clang-format │
 │ • Acceptance  │    │                │
 └──────┬───────┘    └───────┬───────┘
        ▼                    ▼
     ✅ / ❌              ✅ / ❌
        │
        ▼  (after merge)
     dev-tag ← modu-core-v1.2.0-dev.4
```

No action required. Fully automatic.

---

# How To: Feature Release (MINOR bump)

1. Go to **Actions** → **Release**
2. Click **"Run workflow"**
3. In ***"Use workflow from"*** select: **`main`**
4. (Optional) Check **"Dry run"** to preview
5. Click **"Run workflow"**

### What happens automatically

- Bumps MINOR: `v1.1.0` → `v1.2.0`
- Builds Release binary + `.deb` package
- Pushes annotated git tag
- Publishes GitHub Release with changelog

---

# How To: Feature Release — Visual

```
  GitHub Actions UI
  ┌───────────────────────────────┐
  │  Run workflow                 │
  │  Use workflow from: [ main ▼] │
  │  ☐ Dry run                    │
  │  [ Run workflow ]             │
  └───────────────────────────────┘
              │
              ▼
      modu-core-v1.2.0
      ├── modu-core (binary)
      ├── modu-core-1.2.0-Linux.deb
      └── Auto-generated changelog
```

---

# How To: Hotfix — Step 1: Create Release Branch

1. Go to **Actions** → **Create Release Branch**
2. Enter the release tag: `modu-core-v1.2.0` (or leave empty for latest)
3. Run → creates `release/v1.2.x`

### Step 2: Apply the fix

1. Create a branch from `release/v1.2.x`
2. Fix the bug, open a PR targeting `release/v1.2.x`
3. Get review, merge

### Step 3: Release the hotfix

1. Go to **Actions** → **Release**
2. In *"Use workflow from"* select: **`release/v1.2.x`**
3. Run → produces `modu-core-v1.2.1`

---

# How To: Hotfix — Visual

```
 Step 1: Create release branch
 ┌────────────────────────────────────┐
 │ Release tag: [modu-core-v1.2.0  ] │
 │ [ Run workflow ]                  │
 └────────────────────────────────────┘
              │
              ▼
      release/v1.2.x created

 Step 3: Release from hotfix branch
 ┌────────────────────────────────────┐
 │ Use workflow from: [release/v1.2.x▼]
 │ [ Run workflow ]                  │
 └────────────────────────────────────┘
              │
              ▼
      modu-core-v1.2.1
```

---

# Build Artifacts

Every release publishes to GitHub Releases:

| Artifact | Description |
|----------|-------------|
| `modu-core` | Stripped ELF binary |
| `modu-core-X.Y.Z-Linux.deb` | Debian package |
| Changelog | Auto-generated from PR titles |

### Install the .deb

```bash
sudo dpkg -i modu-core-1.2.0-Linux.deb
```

### Application logs its version on startup

```
I20260314 16:52:36.547907 main.cpp:19] Starting modu-core-v1.2.0 (1.2.0)
```

---

# Common Scenarios — Cheat Sheet

| I want to... | Do this |
|-------------|---------|
| Release a new feature version | Actions → Release → select `main` |
| Hotfix a released version | Create release branch → fix via PR → Actions → Release → select `release/vX.Y.x` |
| Preview a release without publishing | Actions → Release → check "Dry run" |
| Bump MAJOR version | Edit `VERSION` file: `MAJOR=2`, merge to main |
| Check which version a commit belongs to | Look at `dev.N` tags on main |

---

# What NOT to Do

❌ **Don't create tags manually** — the CI handles all tagging

❌ **Don't push directly to `main` or `release/*`** — always use PRs

❌ **Don't edit version numbers in CMakeLists.txt** — they come from CI / git describe

❌ **Don't delete release branches** — they're protected and may be needed for future patches

❌ **Don't run Release workflow from a feature branch** — it will fail validation (only `main` and `release/vX.Y.x` are allowed)

---

# Tag Prefixes — Reference

| Pattern | Meaning | Example |
|---------|---------|---------|
| `modu-core-vX.Y.Z` | Release tag | `modu-core-v1.2.0` |
| `modu-core-vX.Y.Z-dev.N` | Dev snapshot | `modu-core-v1.2.0-dev.3` |
| `modu-core-vX.Y.Z-local.N` | Local build | `modu-core-v1.2.0-local.5` |
| `docker_vX.Y.Z` | Devcontainer image | `docker_v1.0.13` |

---

# Summary

✅ **Simple** — two types of releases (feature + hotfix), one workflow each

✅ **Automated** — version numbers, tags, changelogs, packages — all CI-driven

✅ **Safe** — branch protection, PR reviews, no manual tags

✅ **Traceable** — every merge to main gets a dev tag, every release gets a GitHub Release

✅ **Yocto-ready** — bash-sourceable VERSION file, cmake install target, .deb packages

---

# Questions?

📄 Full documentation: `docs/RELEASE_PROCESS.md`

🔗 Repository: [github.com/prze-kiesz/modu-core](https://github.com/prze-kiesz/modu-core)
