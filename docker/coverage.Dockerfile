### Coverage container ###
FROM bingit-assignment-2:base as coverage

# Copy directory to /usr/src/project
COPY . /usr/src/project

# Set build_coverage directory as current working directory
WORKDIR /usr/src/project/build_coverage

# Create coverage reports
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage