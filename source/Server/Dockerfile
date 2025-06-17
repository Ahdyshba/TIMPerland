# Base image with Ubuntu 22.04
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install Qt, build tools, and SQLite
RUN apt-get update && apt-get install -y \
    qtbase5-dev \
    qt5-qmake \
    qtbase5-dev-tools \
    build-essential \
    libsqlite3-dev \
    sqlite3 \
    git \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy all project files (source, .pro, etc.)
COPY . .

# Build the application using qmake + make
RUN qmake && make

# Expose the TCP port used by the server
EXPOSE 55555

# List files to help verify build during image build logs
RUN ls -l /app

# Run the server binary
CMD ["./server"]
