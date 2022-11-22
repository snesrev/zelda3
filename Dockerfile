FROM ubuntu:20.04 as build
ENV TZ=Etc/UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install dependencies
## apt install python & libraries
RUN apt-get update && \
    apt-get install -y \
        python3 \
        python3-pip \
        libjpeg-dev \
        zlib1g-dev \
        libsdl2-dev \
        git \
        wget

## install python dependencies
COPY requirements.txt /tmp/
RUN pip3 install -r /tmp/requirements.txt

## Windows build dependencies
#RUN apt-get install -y \
#        binutils-mingw-w64
#RUN apt-get install -y \
#        tcc \
#        unzip

## Switch build dependencies
### Install devkitpro for switch build
#RUN wget https://apt.devkitpro.org/install-devkitpro-pacman && \
#    chmod +x ./install-devkitpro-pacman && \
#    sed -i 's/apt-get/apt-get -y /g' ./install-devkitpro-pacman && \
#    ./install-devkitpro-pacman
### Install switch development tools
#RUN ln -s /proc/self/mounts /etc/mtab
#RUN dkp-pacman --noconfirm -S switch-dev switch-sdl2 switch-tools

RUN mkdir /zelda3
WORKDIR /zelda3

CMD echo 'usage: docker run --rm --mount type=bind,source="$(pwd)",destination=/zelda3 zelda3 make'