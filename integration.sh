#!/bin/bash
echo "======================================="
echo "Integration tests start"

# start webserver
./bin/server ../configs/8080_config & 
id=$!

sleep 1
# valid request test
expected=$'<!DOCTYPE html><html><body><p>Welcome to the server.</p><p>Would you like to set Bing as your preferred search engine?</p></body></html>'

generated_output=$(curl -sS localhost:8080/static/index.html)

echo -e "\n\nINTEGRATION: running valid request test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo -e "Valid Request Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Valid Request Test Passed\n"
fi

# invalid request test
expected=$(echo -e "HTTP/1.1 400 Bad Request\r\nContent-Length: 31\r\nContent-Type: text/plain\r\n\r\n400 Error: Bad Request Received\\n")

generated_output=$(echo -e 'GE / HTTP/1.1\r\n' | nc localhost 8080 -w1)
 
echo -e "\n\nINTEGRATION: running invalid request test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo -e "Invalid Request Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Invalid Request Test Passed\n"
fi

# bad path test
expected=$(echo -e "HTTP/1.1 404 Not Found\r\nContent-Length: 25\r\nContent-Type: text/plain\r\n\r\n404 Error: Page not found\\n")

generated_output=$(echo -e 'GET /static/doesnt_exist.html HTTP/1.1\r\n' | nc localhost 8080 -w1)

echo -e "\n\nINTEGRATION: running bad path test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo -e "Bad Path Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Bad Path Test Passed\n"
fi

echo test
expected=$'HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 26\r\n\r\nGET /echo/test HTTP/1.1\r\n'
expected=$(echo -e "HTTP/1.1 200 OK\r\nContent-Length: 26\r\nContent-Type: text/plain\r\n\r\nGET /echo/test HTTP/1.1\r\n")

generated_output=$(echo -e 'GET /echo/test HTTP/1.1\r\n' | nc localhost 8080 -w1 )

echo -e "\n\nINTEGRATION: running echo test\n"

if [ "$expected" != "$generated_output" ]; 
then
    echo -e "Echo Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Echo Test Passed\n"
fi

echo "All integration Tests Passed"
echo "======================================="
kill -s SIGINT $id