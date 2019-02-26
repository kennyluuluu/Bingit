#!/bin/bash
echo "======================================="
echo "Thread Integration tests start"

# start webserver
./bin/server ../configs/8080_config & 
id=$!

sleep 1

#multithread test
expected=$(echo -e "HTTP/1.1 200 OK\r\nContent-Length: 26\r\nContent-Type: text/plain\r\n\r\nGET /echo/test HTTP/1.1\r\n")

for i in {1..15}
do
  echo_request=$(curl -s --limit-rate 1 "http://localhost:8080/echo/") &
done

generated_output=$(echo -e 'GET /echo/test HTTP/1.1\r\n' | nc localhost 8080 -w1)

echo -e "\n\nINTEGRATION: running multithread test\n"

if [ "$expected" != "$generated_output" ];
then
    echo "${expected}"
    echo "${generated_output}"
    echo -e "Multithread Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Multithread Test Passed\n"
fi

#status test
for i in {1..10}
do
    static_request=$(curl -sS localhost:8080/static/index.html)
done

for i in {1..3}
do
    static2_request=$(curl -sS localhost:8080/static2/index.html) 
done

for i in {1..7}
do
    bad_request=$(echo -e 'GET /static/doesnt_exist.html HTTP/1.1\r\n' | nc localhost 8080 -w1)
done


expected=$(echo -e "HTTP/1.1 200 OK\r\nContent-Length: 579\r\nContent-Type: text/plain\r\n\r\n\
Total Number of Requests Received: 36\n\n\
Number of requests received for /static/index.html: 10\n\
Number of requests received for /echo/: 15\n\
Number of requests received for /static/doesnt_exist.html: 7\n\
Number of requests received for /static2/index.html: 3\n\
Number of requests received for /echo/test: 1\n\n\
Number of 404 responses sent: 7\n\
Number of 200 responses sent: 29\n\n\
A static request handler exists for the path: /static2\n\
A status request handler exists for the path: /status\n\
A echo request handler exists for the path: /echo\n\
A static request handler exists for the path: /static\n")

generated_output=$(echo -e 'GET /status HTTP/1.1\r\n' | nc localhost 8080 -w1)
if [ "$expected" != "$generated_output" ];
then
    echo "${expected}"
    echo "${generated_output}"
    echo -e "Status Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Status Test Passed\n"
fi

echo "All integration Tests Passed"
echo "======================================="
kill -s SIGINT $id
