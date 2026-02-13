FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    make \
    git \
    zip \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Cache STM32 HAL/CMSIS in image layer (only the needed submodules)
RUN git clone --depth 1 https://github.com/STMicroelectronics/STM32CubeF1.git /opt/STM32CubeF1 \
    && cd /opt/STM32CubeF1 \
    && git submodule update --init --depth 1 \
        Drivers/CMSIS/Device/ST/STM32F1xx \
        Drivers/STM32F1xx_HAL_Driver \
    && rm -rf .git Drivers/BSP Projects Middlewares Utilities docs

RUN git clone --depth 1 https://github.com/STMicroelectronics/STM32CubeF0.git /opt/STM32CubeF0 \
    && cd /opt/STM32CubeF0 \
    && git submodule update --init --depth 1 \
        Drivers/CMSIS/Device/ST/STM32F0xx \
        Drivers/STM32F0xx_HAL_Driver \
    && rm -rf .git Drivers/BSP Projects Middlewares Utilities docs

COPY scripts/docker-build.sh /usr/local/bin/docker-build.sh
RUN chmod +x /usr/local/bin/docker-build.sh

WORKDIR /work

ENTRYPOINT ["/usr/local/bin/docker-build.sh"]
