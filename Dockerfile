FROM ubuntu:18.04

WORKDIR /opt/zap

COPY cmake-build-debug/bin .
COPY cmake-build-debug/lib .

RUN [ "./bin/zaprt", "./lib/libip_handler.so" ]