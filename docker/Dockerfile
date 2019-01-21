# ### Build/test container ###
# 
### Base environment container ###
# Get the base Ubuntu image from Docker Hub
FROM ubuntu:latest as builder

# Update the base image and install build environment
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    libboost-log-dev \
    libboost-signals-dev \
    libboost-system-dev \
    libgtest-dev \
    gcovr

COPY . /usr/src/project
WORKDIR /usr/src/project/build

RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
FROM ubuntu:latest as deploy

# TODO(!): Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/server .

# Copy config files
COPY configs/* conf/

# TODO(!): Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
# TODO(!): Update with real server name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
# TODO(!): Update with real server args
CMD ["/conf/my_config"]
