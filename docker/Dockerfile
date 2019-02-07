### Build/test container ###
FROM bingit-assignment-2:base as builder

# Copy directory to /usr/src/project
COPY . /usr/src/project

# Set build directory as current working directory
WORKDIR /usr/src/project/build

#build executable and run tests
RUN cmake ..

RUN make

RUN ctest -V --output-on_failure

### Deploy container ###
FROM ubuntu:latest as deploy

# Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/server .

# Copy config files
COPY configs/* configs/

# Copy static folder and files
COPY static_roots/ .

# Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["/configs/my_config"]
