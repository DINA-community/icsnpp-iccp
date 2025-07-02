FROM debian:bookworm

RUN apt-get update && \
    apt-get install -y cmake build-essential

ADD libiec61850-1.5.1.tar.gz /root/

WORKDIR /root/libiec61850-1.5.1

RUN mkdir build && cd build && \
    cmake .. && \
    make && make install && \
    echo "/usr/local/lib/" >> /etc/ld.so.conf.d/local.conf && \
    ldconfig
