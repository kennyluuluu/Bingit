#!/bin/bash
echo "======================================="
echo "Integration tests start"

# start webserver
./bin/server ../configs/8080_config & 
id=$!

# valid request test
expected=$'<!DOCTYPE html><html><body><p>Welcome to the server.</p><p>Would you like to set Bing as your preferred search engine?</p></body></html>'

generated_output=$(curl -sS localhost:8080)

echo -e "\n\nINTEGRATION: running valid request test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "Valid Request Test Failed"
    kill -s SIGINT $id
    exit 1
fi

# invalid request test
expected=$'HTTP/1.1 400 Bad Request\r\n\r'
expected=$(echo -e "$expected" | od -c)

generated_output=$(echo -e 'GE / HTTP/1.1\r\n' | nc localhost 8080 -w1 | od -c)

echo -e "\n\nINTEGRATION: running invalid request test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "Invalid Request Test Failed"
    kill -s SIGINT $id
    exit 1
fi

# bad path test
expected=$'404 Error: Page not found\r'
expected=$(echo "$expected" | od -c)

generated_output=$(curl -sS localhost:8080/static/non_existent_file | od -c)

echo -e "\n\nINTEGRATION: running bad path test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "Bad Path Test Failed"
    kill -s SIGINT $id
    exit 1
fi

# echo test
expected=$'HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 26\r\n\r\nGET /echo/test HTTP/1.1\r\n'
expected=$(echo "$expected" | od -c)

generated_output=$(echo -e 'GET /echo/test HTTP/1.1\r\n' | nc localhost 8080 -w1 | od -c)

echo -e "\n\nINTEGRATION: running echo test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "Echo Test Failed"
    kill -s SIGINT $id
    exit 1
fi

echo "Test Passed"
echo "======================================="
kill -s SIGINT $id