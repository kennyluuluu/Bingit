#!/bin/bash
echo "======================================="
echo "Thread Integration tests start"

PATH_TO_CURL_OUTPUT="../tests/curloutput"
PATH_TO_CURL_EXPECTED="../tests/curloutput_expected"

# start webserver
./bin/server ../configs/8080_config & 
id=$!

sleep 1

#multithread test
for i in {1..15}
do
  echo_request=$(curl -s -H "Keep-Alive: 3" "http://localhost:8080/echo/") &
done

curl -s "http://localhost:8080/echo/" -o $PATH_TO_CURL_OUTPUT
# User-Agent: curl/7.47.0 on devel environment, User-Agent: curl/7.58.0 in cloud, regex diff now ignores this line
diff --ignore-matching-lines '.*User\-Agent:.*' $PATH_TO_CURL_OUTPUT $PATH_TO_CURL_EXPECTED
rc=$?;

echo -e "\n\nINTEGRATION: running multithread test\n"

if [ $rc -ne 0 ] 
then
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


expected=$(echo -e "HTTP/1.1 200 OK\r\nContent-Length: 637\r\nContent-Type: text/plain\r\n\r\n\
Total Number of Requests Received: 36\n\n\
Number of requests received for /echo/: 16\n\
Number of requests received for /static/doesnt_exist.html: 7\n\
Number of requests received for /static/index.html: 10\n\
Number of requests received for /static2/index.html: 3\n\n\
Number of 200 responses sent: 29\n\
Number of 404 responses sent: 7\n\n\
A echo request handler exists for the path: /echo\n\
A health request handler exists for the path: /health\n\
A meme request handler exists for the path: /meme\n\
A static request handler exists for the path: /static\n\
A static request handler exists for the path: /static2\n\
A status request handler exists for the path: /status\n")

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
