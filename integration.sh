#!/bin/bash

# start webserver
./bin/server ../configs/8080_config &
id=$!

expected=$'GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.58.0\r\nAccept: */*\r\n\r'

# valid request test
generated_output=$(curl -sS localhost:8080)

if [ "$expected" != "$generated_output" ]; 
then
    echo "Test Failed"
    kill -s SIGINT $id
    exit 1
fi

expected=$'HTTP/1.1 400 Bad Request\r\n\r'
expected=$(echo "$expected" | od -c)

# invalid request test
generated_output=$(echo -e 'GE / HTTP/1.1\r\n' | nc localhost 8080 -w1 | od -c) # | tr -d '\r')

# echo "$generated_output"

if [ "$expected" != "$generated_output" ]; 
then
    echo "Test Failed"
    kill -s SIGINT $id
    exit 1
fi

echo "Test Passed"
kill -s SIGINT $id