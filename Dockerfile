FROM ubuntu:18.04

WORKDIR /opt/zap

COPY cmake-build-debug/bin .

EXPOSE 9993

#CMD ["/bin/bash", "-c", "/opt/zap/zaprt /opt/zap/lib/libip_handler.so" ]

CMD [ "/opt/zap/zaprt" ]
