FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y openssl

WORKDIR /opt/zap

COPY cmake-build-debug/bin .
COPY cmake-build-debug/lib/libzap.so .

RUN ls

EXPOSE 9993

#CMD ["/bin/bash", "-c", "/opt/zap/zaprt /opt/zap/lib/libip_handler.so" ]

ENV LD_LIBRARY_PATH="/opt/zap"
CMD [ "/opt/zap/zaprt" ]
