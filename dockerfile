FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Update, install dependencies, and clean up in one layer
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git \
    qt6-base-dev qt6-declarative-dev qt6-tools-dev qt6-tools-dev-tools \
    qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtqml-workerscript \
    libxcb-cursor0 libxcb-cursor-dev libxcb-image0-dev libxcb-render-util0-dev libxcb-render0-dev libxcb-shm0-dev \
    python3 python3-dev python3-pip python3-flask \
    usbutils && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

RUN chmod +x build.sh && \
    ./build.sh

EXPOSE 10000-20000

CMD ["./build/Application"]
