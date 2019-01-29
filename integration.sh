#!/bin/bash

# start webserver
./bin/server ../configs/8080_config &
id=$!

expected=$'GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.58.0\r\nAccept: */*\r\n\r'

# test
generated_output=$(curl -sS localhost:8080)

if [ "$expected" == "$generated_output" ]; then
    echo "Test Passed"
    kill -s SIGINT $id
else
    echo "Test Failed"
    kill -s SIGINT $id
    exit 1
fi

