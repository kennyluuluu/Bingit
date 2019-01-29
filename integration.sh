#!/bin/bash

# start webserver
./build/bin/server ./configs/8080_config &
id=$?

expected=$'GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.58.0\r\nAccept: */*\r\n\r'

# test
generated_output=$(curl -sS localhost:8080)

#echo $generated_output
if [ "$expected" == "$generated_output" ]; then
    echo "Test Passed"
else
    echo "Test Failed"
fi

# stop webserver
kill -s SIGINT $id