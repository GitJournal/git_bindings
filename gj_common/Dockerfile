FROM ubuntu:18.10

ENV DEBIAN_FRONTEND noninteractive
ENV TZ Europe/Madrid

RUN apt-get update
RUN apt-get install -y libssh2-1-dev curl gdb cppcheck xz-utils sudo libtinfo5
RUN apt-get install -y cmake libssl-dev pkg-config zlib1g-dev clang tzdata git vim

COPY . /opt
RUN cd /opt && ./build-libgit2.sh
