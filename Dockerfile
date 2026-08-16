# MiniRedis Container Image
FROM msys2/msys2:latest

# Install C++ dependencies
RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm mingw-w64-ucrt-x86_64-gcc \
                          mingw-w64-ucrt-x86_64-cmake \
                          mingw-w64-ucrt-x86_64-ninja

WORKDIR /app
COPY . .

# Build MiniRedis
RUN export PATH=/ucrt64/bin:$PATH && \
    cmake -B build -S . -G Ninja && \
    cmake --build build

EXPOSE 6379

CMD ["./build/miniredis_server", "0.0.0.0", "6379"]
