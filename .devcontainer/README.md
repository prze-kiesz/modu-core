# Docker Development Container

This document describes the Docker-based development environment for modu-core.

## Overview

The development container provides a fully configured Ubuntu 24.04 environment with all necessary tools and dependencies for building and testing modu-core.

## Quick Start

### Using Prebuilt Image

The fastest way to start developing is using the prebuilt image from GitHub Container Registry:

```bash
# Pull the latest image
docker pull ghcr.io/prze-kiesz/modu-core:latest
```

**VS Code Integration:**
1. Open the project in VS Code
2. Install the "Dev Containers" extension
3. Edit `.devcontainer/devcontainer.json` and uncomment the `"image"` line
4. Press `F1` → "Dev Containers: Reopen in Container"

### Building Locally

If you need to customize the container or don't have access to GitHub Container Registry:

```bash
cd .devcontainer
docker build -t modu-core-dev .
```

## Container Contents

### Base System
- **Ubuntu 24.04 LTS** - Latest long-term support release
- **Locales**: en_US.UTF-8

### Compilers & Build Tools
- **GCC** - GNU Compiler Collection (latest from Ubuntu repos)
- **Clang 19** - Latest LLVM/Clang toolchain
  - clang, clang++
  - clang-tidy (static analysis)
  - clang-format (code formatting)
  - clangd (language server)
- **CMake** - Latest version from Ubuntu repos
- **Make** - GNU Make
- **ccache** - Compiler cache for faster rebuilds

### Libraries
- **Google Test 1.16.0** (gtest + gmock) - Built from source
- **Google glog 0.6.0** - Logging library, built from source
- **systemd development libraries** - For systemd integration

### Development Tools
- **Git** - Version control
- **pkg-config** - Library configuration tool
- **Vim** - Text editor
- **SSH client** - For remote access
- **Cross-compilation tools** - ARM64 toolchain (crossbuild-essential-arm64)

### Utilities
- curl, wget - Download tools
- unzip - Archive extraction
- autoconf, automake, libtool - Build system tools

## Available Tags

The Docker image is automatically built and tagged by GitHub Actions with semantic versioning.

### Version Management

Version format: `x.y.z` where:
- `x.y` - Major.Minor version stored in `.devcontainer/VERSION` file
- `z` - Patch number, auto-incremented based on existing git tags

**To bump version:**
1. Edit `.devcontainer/VERSION` (e.g., change `1.0` to `1.1`)
2. Commit and merge to main
3. Next build will start at `1.1.0`

### Main Branch Tags

When changes are merged to `main`:
- `latest` - Always points to the newest build
- `x.y.z` - Full semantic version (e.g., `1.0.3`)
- `x.y` - Minor version, tracks latest patch (e.g., `1.0` → `1.0.3`)
- `main` - Main branch identifier
- `sha-abc123` - Specific commit hash
- **Git tag created:** `vx.y.z` (e.g., `v1.0.3`)

**Example progression:**
```
VERSION=1.0, first build  → 1.0.0, 1.0, latest, main, sha-xxx
VERSION=1.0, second build → 1.0.1, 1.0, latest, main, sha-yyy
VERSION=1.1, first build  → 1.1.0, 1.1, latest, main, sha-zzz
```

### Branch Tags

Feature branches (e.g., `feature/new-module`):
- `feature-new-module` - Branch name (slashes become hyphens)
- `sha-def456` - Commit hash

### Pull Request Tags

Pull requests (e.g., PR #7):
- `pr-7` - PR number
- `sha-ghi789` - Commit hash

**Note:** PR images are built but not pushed to registry (build verification only).

### Multi-Architecture Support

Currently supports: `linux/amd64`

**To enable multi-arch builds**, edit `.github/workflows/docker-build.yml`:
```yaml
platforms: linux/amd64,linux/arm64,linux/arm/v7,linux/riscv64
```

**Using multi-arch images:**
```bash
# Docker automatically pulls the correct architecture
docker pull ghcr.io/prze-kiesz/modu-core:latest

# Explicitly specify platform
docker pull --platform linux/arm64 ghcr.io/prze-kiesz/modu-core:1.0.0

# View available architectures
docker manifest inspect ghcr.io/prze-kiesz/modu-core:latest
```

## Image Metadata

The image includes OCI-compliant labels:

```dockerfile
org.opencontainers.image.created       - Build timestamp
org.opencontainers.image.authors       - Maintainer information
org.opencontainers.image.url           - Project homepage
org.opencontainers.image.source        - GitHub repository
org.opencontainers.image.version       - Version number
org.opencontainers.image.revision      - Git commit SHA
org.opencontainers.image.title         - Image name
org.opencontainers.image.description   - Brief description
```

View metadata:
```bash
docker inspect ghcr.io/prze-kiesz/modu-core:latest | jq '.[0].Config.Labels'
```

## Customization

### Build Arguments

The Dockerfile accepts build arguments for versioning:

```bash
docker build \
  --build-arg VERSION=1.0.0 \
  --build-arg BUILD_DATE=$(date -u +'%Y-%m-%dT%H:%M:%SZ') \
  --build-arg VCS_REF=$(git rev-parse --short HEAD) \
  -t modu-core-dev:1.0.0 \
  .devcontainer
```

### Adding Dependencies

To add new dependencies:

1. Edit `.devcontainer/Dockerfile`
2. Add installation steps using `apt install` or build from source
3. Update this documentation
4. Commit changes - GitHub Actions will automatically build new image

### VS Code Extensions

The container includes pre-configured VS Code extensions (defined in `devcontainer.json`):

- **C/C++ Extension Pack** - IntelliSense, debugging, formatting
- **CMake Tools** - CMake integration
- **Docker** - Docker file support
- **Buf** - Protobuf support
- **Makefile Tools** - Makefile syntax
- **clangd** - C++ language server
- **GitHub Copilot** - AI pair programming (requires subscription)

## Updating the Container

### Manual Updates

```bash
# Rebuild the image
cd .devcontainer
docker build --no-cache -t modu-core-dev .

# If using prebuilt image, pull latest
docker pull ghcr.io/prze-kiesz/modu-core:latest

# Restart VS Code container
# F1 → "Dev Containers: Rebuild Container"
```

### Automatic Updates

GitHub Actions automatically builds and publishes new images when:
- Changes are pushed to `.devcontainer/Dockerfile` or `devcontainer.json`
- A new release is published
- Manually triggered via workflow_dispatch

## Troubleshooting

### Container Won't Start

```bash
# Check Docker daemon
sudo systemctl status docker

# Check logs
docker logs <container-id>

# Remove old containers
docker container prune
```

### Out of Disk Space

```bash
# Clean up unused images
docker image prune -a

# Remove build cache
docker builder prune
```

### Permission Issues

The container runs as user `ubuntu` with passwordless sudo access. SSH keys are mounted read-only from your host `~/.ssh` directory.

### Build Failures

If image build fails:
1. Check Dockerfile syntax
2. Verify network connectivity (for downloading dependencies)
3. Check GitHub Actions logs for detailed error messages
4. Try building locally with `--no-cache` flag

## CI/CD Integration

The Docker image build is integrated with GitHub Actions:

**Workflow**: `.github/workflows/docker-build.yml`

**Triggers**:
- Push to main branch (changes to `.devcontainer/`)
- Pull requests (changes to `.devcontainer/`)
- Release publication
- Manual dispatch

**Build Process**:
1. Checkout repository
2. Set up Docker Buildx (multi-platform support)
3. Login to GitHub Container Registry
4. Extract metadata (tags, labels)
5. Build image with cache from previous builds
6. Push to `ghcr.io/prze-kiesz/modu-core` (on main branch)

**Caching**: GitHub Actions cache is used to speed up subsequent builds.

## Security

- Container runs as non-root user (`ubuntu`)
- Minimal attack surface - only essential tools installed
- Regular base image updates (Ubuntu 24.04 LTS receives security updates)
- Dependencies built from specific tagged versions (not `latest`)

## Performance

**Image Size**: ~2-3 GB (due to clang-19, build tools, and libraries)

**Build Time**:
- From scratch: ~15-20 minutes (building gtest, glog from source)
- With cache: ~2-5 minutes

**Optimization Tips**:
- Use prebuilt image from ghcr.io instead of building locally
- Enable ccache for faster C++ compilation
- Use Docker Buildx cache for incremental builds

## Contributing

When modifying the development container:

1. **Test locally first**: Build and test the Dockerfile changes locally
2. **Update documentation**: Update this file and README.md
3. **Increment version**: Update VERSION in Dockerfile and devcontainer.json
4. **Submit PR**: GitHub Actions will build but not push the image
5. **After merge**: Image is automatically built and published to ghcr.io

## Resources

- [VS Code Dev Containers](https://code.visualstudio.com/docs/devcontainers/containers)
- [GitHub Container Registry](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry)
- [Docker Build Best Practices](https://docs.docker.com/develop/dev-best-practices/)
- [OCI Image Spec](https://github.com/opencontainers/image-spec)

---

**Last Updated**: January 5, 2026  
**Maintainer**: Przemek Kieszkowski
