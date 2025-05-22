# Use Ubuntu as base
FROM ubuntu:22.04

# Set environment variables to avoid interaction prompts
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
    && apt-get clean

# Set working directory
WORKDIR /app

# Copy your project files into the container
COPY . .

# Build the application using qmake and make
RUN qmake && make

# Expose the TCP port
EXPOSE 55555

# List the contents of /app to verify the executable
RUN ls -l /app

# Run the compiled server
CMD ["./server"]
