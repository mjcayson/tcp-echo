#builder: toolchain + gtest + build
FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git pkg-config \
    libgtest-dev \
    clang-tidy cppcheck gdb \
 && rm -rf /var/lib/apt/lists/*

# Build & install GoogleTest to /usr/local (Ubuntu ships sources in /usr/src/googletest)
RUN cmake -S /usr/src/googletest -B /tmp/build-gtest -DBUILD_GMOCK=ON \
 && cmake --build /tmp/build-gtest -j \
 && cmake --install /tmp/build-gtest

WORKDIR /src
COPY . /src

# Configure with tests ON for builder image (so ctest is available)
RUN cmake -S . -B /build \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DECHO_BUILD_TESTS=ON \
      -DECHO_WARNINGS_AS_ERRORS=OFF
RUN cmake --build /build -j

#runtime: lean image with just the apps
FROM ubuntu:22.04 AS runtime
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 ca-certificates netcat-openbsd \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /build/apps/server/server /app/server
COPY --from=builder /build/apps/client/client /app/client

# default command just prints help
CMD ["/app/server", "--help"]
