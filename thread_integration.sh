#!/bin/bash
echo "======================================="
echo "Thread Integration tests start"

# start webserver
./bin/server ../configs/8080_config > /dev/null & 
id=$!

sleep 2

#multithread test
expected=$(echo -e "HTTP/1.1 200 OK\r\nContent-Length: 26\r\nContent-Type: text/plain\r\n\r\nGET /echo/test HTTP/1.1\r\n")

echo -e "\n\nINTEGRATION: running multithread test\n"

rm -f finished_request.txt
touch finished_request.txt

#should eventually check the validity of this file
rm -f finished_request_output.txt
touch finished_request_output.txt

echo -e "\nStarting partial request"
curl -s --limit-rate 10k "http://localhost:8080/teamcherry" > finished_request_output.txt && echo done > finished_request.txt &
id2=$!
#give request some time to transfer data
sleep 2

#stop the request
kill -STOP $id2
echo -e "\nFull Request\n--------------------------"
echo "Started"
generated_output=$(echo -e 'GET /echo/test HTTP/1.1\r\n' | nc localhost 8080 -w1)
echo "Finished"
echo "--------------------------"

#restart request
kill -CONT $id2
echo -e "\nRestarting partial request\n"

#while the finished_request.txt file is empty
while ! [ -s finished_request.txt ]
do
    #spin on loop while request is finishing
    echo "Waiting for curl to finish..."
    sleep 1
done

echo -e "\nFinished partial request\n"

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

rm -f finished_request.txt

echo -e "\n\nINTEGRATION: running status test\n"

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


expected=$(echo -e "HTTP/1.1 200 OK\r\nContent-Length: 640\r\nContent-Type: text/plain\r\n\r\n\
Total Number of Requests Received: 22\n\n\
Number of requests received for /echo/test: 1\n\
Number of requests received for /static/doesnt_exist.html: 7\n\
Number of requests received for /static/index.html: 10\n\
Number of requests received for /static2/index.html: 3\n\
Number of requests received for /teamcherry: 1\n\n\
Number of 404 responses sent: 7\n\
Number of 200 responses sent: 15\n\n\
A proxy request handler exists for the path: /teamcherry\n\
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
