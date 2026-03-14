# Release Process

This document describes the versioning scheme and release lifecycle for modu-core.

---

## GitHub Actions — overview

| Workflow | Trigger | Purpose |
|----------|---------|---------|
| **build-and-test.yml** | Push / PR to `main`, `develop`; Docker image rebuilt | Builds Debug + Release, runs unit tests (ctest) and acceptance tests (pytest). Gate for every merge. |
| **static-analysis.yml** | Push / PR touching C++ or CMake files | Runs clang-tidy, cppcheck and clang-format checks. Reports uploaded as artifacts. |
| **docker-build.yml** | Changes to `.devcontainer/`; manual | Builds and pushes the devcontainer image to GHCR. Tags with `docker_vX.Y.Z`. |
| **dev-tag.yml** | Every push to `main` (auto) | Creates a lightweight `modu-core-vX.Y.Z-dev.N` tag for traceability of every merge. |
| **release.yml** | Manual (`workflow_dispatch`) | Unified release workflow — MINOR bump from `main`, PATCH bump from `release/vX.Y.x`. Builds, packages `.deb`, pushes annotated tag, publishes GitHub Release. |
| **create-release-branch.yml** | Manual (`workflow_dispatch`) | Creates a `release/vX.Y.x` branch from an existing release tag for hotfix work. |

---

## Branching model

```
        main                          
          │
          ● merge PR
          │ ← dev.1
          │
          ● merge PR
          │ ← dev.2
          │
          ● merge PR
          │ ← dev.3
          │
          ● Release (MINOR updated)
          │ ← v1.1.0
          │
          ● merge PR
          │ ← dev.4
          │
          ● merge PR
          │ ← dev.5
          │
          ● Release (MINOR updated)
          │ ← v1.2.0 --create-release-branch--> release/v1.2.x
          │                                          │
          │                                          │
          │                                          ● hotfix PR
          │                                          │
          │                                          ● Release (PATCH)
          │                                          │ ← v1.2.1
          │                                          │
          │                                          ● hotfix PR
          │                                          │
          │                                          ● Release (PATCH)
          │                                          │ ← v1.2.2
          │                                          │
          ● merge PR                                 ▼
          │ ← dev.6
          │
          ▼
```

**Key rules:**

- **`main`** — all feature development lands here via PRs. Every merge gets a `dev.N` tag automatically.
- **Feature release** — run Release workflow on `main` → creates `vX.(Y+1).0` tag + GitHub Release.
- **`release/vX.Y.x`** — created from a release tag when hotfixes are needed. Protected like `main` (PR required).
- **Hotfix** — commit fix to `release/vX.Y.x` via PR, then run Release workflow → creates `vX.Y.(Z+1)` tag.
- Tags are **never** created manually — always through GitHub Actions.

---

## Version scheme

```
modu-core-vMAJOR.MINOR.PATCH[-dev.N]
```

| Component | Who sets it | When |
|-----------|-------------|------|
| `MAJOR` | Developer — edit `VERSION` file | Breaking change, major milestone |
| `MINOR` | CI auto-increment | Every feature release from `main` |
| `PATCH` | CI auto-increment | Every hotfix release from `release/vX.Y.x` |
| `dev.N` | CI auto-increment | Every merge to `main` (between releases) |

### Single source of truth

`VERSION` file in the repo root:
```bash
MAJOR=1
```

- CMake reads it via `-DMODU_CORE_VERSION=modu-core-vX.Y.Z` (passed by CI)
- Yocto recipes source it directly (`source VERSION && echo $MAJOR`)
- Local builds fall back to `modu-core-v0.0.0`

---

## Tag prefixes

| Pattern | Meaning |
|---------|---------|
| `modu-core-vX.Y.Z` | Release tag (GitHub Release + .deb) |
| `modu-core-vX.Y.Z-dev.N` | Development snapshot (no GitHub Release) |
| `docker_vX.Y.Z` | Devcontainer image (separate from app versioning) |

---

## Lifecycle flows

### 1. Normal development — dev tags

Every PR merged to `main` triggers `dev-tag.yml` automatically:

```
PR merged → main
  └── modu-core-v1.0.0-dev.1
PR merged → main
  └── modu-core-v1.0.0-dev.2
...
```

No action required. Provides full traceability of every `main` commit.

---

### 2. Feature release — MINOR bump

Run **Actions → Release** with `branch = main`:

```
Actions → Release (branch=main)
  ├── finds highest modu-core-v1.*.0 tag
  ├── MINOR = highest + 1
  ├── builds Release binary + .deb
  ├── pushes annotated tag: modu-core-v1.2.0
  └── GitHub Release with changelog since previous release
```

Changelog is auto-generated from PR titles and labels (see `.github/release.yml`).

---

### 3. Hotfix — PATCH bump

**Step 1:** Create a release branch (once per minor version):

```
Actions → Create Release Branch (release_tag=modu-core-v1.2.0)
  └── creates branch: release/v1.2.x
```

**Step 2:** Apply the fix on `release/v1.2.x` (via PR or direct commit).

**Step 3:** Run **Actions → Release** with `branch = release/v1.2.x`:

```
Actions → Release (branch=release/v1.2.x)
  ├── finds highest modu-core-v1.2.* tag
  ├── PATCH = highest + 1
  ├── builds Release binary + .deb
  ├── pushes annotated tag: modu-core-v1.2.1
  └── GitHub Release with changelog
```

To always target the latest release branch without looking it up manually,
use `branch = latest-release-branch`.

---

### 4. MAJOR bump

Edit `VERSION` manually:
```bash
# VERSION
MAJOR=2
```

Commit and merge to `main`. The next release from `main` will produce `modu-core-v2.1.0`
(MINOR resets because CI searches for the highest `v2.*.0` tag — which starts at 0).

---

## Release workflow inputs

**Actions → Release** (`release.yml`):

| Input | Values | Effect |
|-------|--------|--------|
| `branch` | `main` | MINOR bump from main |
| `branch` | `latest-release-branch` | PATCH bump, auto-detects latest `release/vX.Y.x` |
| `branch` | `release/vX.Y.x` | PATCH bump from specific branch |
| `dry_run` | `true` | Computes and logs tag, does not push or publish |

---

## Build artifacts

Every release produces:

| Artifact | Description |
|----------|-------------|
| `modu-core` | Stripped ELF binary |
| `modu-core-X.Y.Z-Linux.deb` | Debian package with runtime deps |

Runtime dependencies declared in the .deb:
```
libgoogle-glog0v6 | libgoogle-glog0t64, libsystemd0
```

---

## cmake --install (Yocto)

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DMODU_CORE_VERSION=modu-core-v1.2.0

cmake --build build -j$(nproc)
cmake --install build --prefix /usr
# installs: /usr/bin/modu-core
```

Yocto `do_install` runs `cmake --install` using the standard `cmake` bbclass.

---

## Changelog categories

Configured in `.github/release.yml`. Labels on PRs control which section a change appears in:

| Label | Section |
|-------|---------|
| `feat`, `feature`, `enhancement` | 🚀 New Features |
| `bug`, `fix`, `hotfix` | 🐛 Bug Fixes |
| `test`, `testing` | 🧪 Tests |
| `ci`, `build` | ⚙️ CI / Build |
| `documentation`, `docs` | 📚 Documentation |
| `chore`, `refactor`, `dependencies` | 🔧 Maintenance |
| `skip-changelog` | (excluded from changelog) |
