# Base image: Latest Ubuntu LTS (24.04)
FROM ubuntu:24.04

# Build arguments for versioning
ARG BUILD_DATE
ARG VCS_REF
ARG VERSION=1.0.0

ENV TERM=xterm-color
ENV DEBIAN_FRONTEND=noninteractive

# OCI labels for container metadata
LABEL org.opencontainers.image.created="${BUILD_DATE}"
LABEL org.opencontainers.image.authors="Przemek Kieszkowski <przemek.kieszkowski@gmail.com>"
LABEL org.opencontainers.image.url="https://github.com/prze-kiesz/modu-core"
LABEL org.opencontainers.image.documentation="https://github.com/prze-kiesz/modu-core/blob/main/README.md"
LABEL org.opencontainers.image.source="https://github.com/prze-kiesz/modu-core"
LABEL org.opencontainers.image.version="${VERSION}"
LABEL org.opencontainers.image.revision="${VCS_REF}"
LABEL org.opencontainers.image.vendor="Przemek Kieszkowski"
LABEL org.opencontainers.image.title="modu-core Development Container"
LABEL org.opencontainers.image.description="Linux build environment with C++20, CMake, glog, GoogleTest, clang-19"

# Legacy labels for compatibility
LABEL maintainer="przemek.kieszkowski@gmail.com"
LABEL version="${VERSION}"
LABEL description="Linux build environment with C++20, CMake, glog, GoogleTest, clang-19"

# Update and install basic build tools and dependencies
RUN apt update && apt upgrade -y && apt install -y --no-install-recommends \
    build-essential \
    gcc \
    g++ \
    cmake \
    make \
    git \
    wget \
    curl \
    pkg-config \
    vim \
    sudo \
    ca-certificates \
    autoconf \
    automake \
    libtool \
    unzip \
    crossbuild-essential-arm64 \
    openssh-client 

# Install additional tools and CLI utilities
RUN apt update && apt upgrade -y && apt install -y --no-install-recommends \
    ccache \
    lsb-release wget software-properties-common gnupg libsystemd-dev \
    jq \
    tree \
    htop \
    file \
    less \
    iputils-ping \
    netcat-openbsd \
    dnsutils

# Install GitHub CLI (gh)
RUN curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg && \
    chmod go+r /usr/share/keyrings/githubcli-archive-keyring.gpg && \
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | tee /etc/apt/sources.list.d/github-cli.list > /dev/null && \
    apt update && apt install -y gh

# Install yq (YAML processor) with version pinning and checksum verification
ARG YQ_VERSION=v4.44.3
RUN wget -q https://github.com/mikefarah/yq/releases/download/${YQ_VERSION}/yq_linux_amd64 -O /usr/local/bin/yq && \
    wget -q https://github.com/mikefarah/yq/releases/download/${YQ_VERSION}/checksums_sha256.txt -O /tmp/yq_checksums_sha256.txt && \
    (cd /usr/local/bin && sha256sum yq | grep "$(grep 'yq_linux_amd64' /tmp/yq_checksums_sha256.txt | cut -d ' ' -f1)") && \
    chmod +x /usr/local/bin/yq && \
    rm /tmp/yq_checksums_sha256.txt

# Set locales
RUN apt update && apt install -y locales && \
    locale-gen en_US.UTF-8 && update-locale LANG=en_US.UTF-8

ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

# Install clang-19 tools
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && ./llvm.sh 19 all && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100 && \
    update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-19 100 && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-19 100 && \
    update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-19 100


# Build and install toml11 (header-only TOML v1.0.0 library) from source
RUN git clone --branch v3.8.1 https://github.com/ToruNiina/toml11.git /tmp/toml11 && \
    cd /tmp/toml11 && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 -DTOML11_INSTALL=ON && \
    cmake --install build && \
    rm -rf /tmp/toml11

# Build and install gRPC and grpc_cli from source
#RUN git clone --recurse-submodules -b v1.60.1 https://github.com/grpc/grpc /tmp/grpc && \
#    cd /tmp/grpc && \
#    mkdir -p cmake/build && cd cmake/build && \
#    cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 -DBUILD_SHARED_LIBS=ON && \
#    make -j"$(nproc)" && \
#    make install && ldconfig

# Build grpc_cli as statically linked binary
#RUN cd /tmp/grpc/cmake/build && rm -rf * &&\
#    cmake -DCMAKE_BUILD_TYPE=Release -DgRPC_BUILD_TESTS=ON -DCMAKE_CXX_STANDARD=17 ../.. && \
#    make -j"$(nproc)" grpc_cli && \
#    cp grpc_cli /usr/local/bin && \
#    cd / && rm -rf /tmp/grpc

# Build and install ALL Boost libraries from source
#RUN mkdir -p /tmp/boost && cd /tmp && \
#    curl -L -o boost.tar.gz https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.gz && \
#    tar -xzf boost.tar.gz && \
#    cd boost_1_88_0 && \
#    ./bootstrap.sh && \
#    ./b2 install -j"$(nproc)" cxxflags="-std=c++17" variant=release link=shared threading=multi && \
#    ldconfig && \
#    cd / && rm -rf /tmp/boost

# Build and install gflags from source
#RUN mkdir -p /tmp/gflags && cd /tmp && \
#    git clone --branch v2.2.2 https://github.com/gflags/gflags.git && \
#    mkdir -p gflags/build && cd gflags/build && \
#    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON && \
#    make -j"$(nproc)" && make install && ldconfig && \
#    cd / && rm -rf /tmp/gflags

# Build and install glog from source
RUN mkdir -p /tmp/glog && cd /tmp && \
    git clone --branch v0.6.0 https://github.com/google/glog.git && \
    mkdir -p glog/build && cd glog/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
    -DBUILD_EXAMPLES=OFF -DWITH_GFLAGS=ON -DWITH_GTEST=OFF && \
    make -j"$(nproc)" && make install && ldconfig && \
    cd / && rm -rf /tmp/glog

# Build and install GoogleTest (gtest + gmock) from source
RUN git clone --branch v1.16.0 https://github.com/google/googletest.git /tmp/googletest && \
    cd /tmp/googletest && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_GMOCK=ON && \
    cmake --build build -j"$(nproc)" && \
    cmake --install build && \
    ldconfig && \
    rm -rf /tmp/googletest
    
# do not require password for sudo
RUN  echo "ubuntu ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER ubuntu
WORKDIR /home/ubuntu
