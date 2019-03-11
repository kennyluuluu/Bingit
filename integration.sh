#!/bin/bash
echo "======================================="
echo "Integration tests start"

# start webserver
./bin/server ../configs/8080_config > /dev/null & 
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
expected=$(echo -e "HTTP/1.1 400 Bad Request\r\nContent-Length: 15\r\nContent-Type: text/plain\r\n\r\n400 Bad Request\\n")

generated_output=$(echo -e 'GE / HTTP/1.1\r\n' | nc localhost 8080 -w1)
 
echo -e "\n\nINTEGRATION: running invalid request test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "${expected}"
    echo "${generated_output}"
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

expected=$(echo -e "HTTP/1.1 200 OK\r\nContent-Length: 26\r\nContent-Type: text/plain\r\n\r\nGET /echo/test HTTP/1.1\r\n")

generated_output=$(echo -e 'GET /echo/test HTTP/1.1\r\n' | nc localhost 8080 -w1 )

echo -e "\n\nINTEGRATION: running echo test\n"

if [ "$expected" != "$generated_output" ]; 
then
    echo "${expected}"
    echo "${generated_output}"
    echo -e "Echo Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Echo Test Passed\n"
fi

expected=$(cat ../integration/new_meme_response)

generated_output=$(echo -e "GET /meme/new HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 )

echo -e "\n\nINTEGRATION: running new meme test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "${expected}"
    echo "${generated_output}"
    echo -e "New Meme Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "New Meme Test Passed\n"
fi

#create memes
echo -e "GET /meme/create HTTP/1.1\r\n\r\nimage=a&top=b&bottom=space+test" | nc localhost 8080 -w1 
echo -e "GET /meme/create HTTP/1.1\r\n\r\nimage=a&top=b&bottom=space+test" | nc localhost 8080 -w1 
echo -e "GET /meme/create HTTP/1.1\r\n\r\nimage=b&top=different&bottom=%20space" | nc localhost 8080 -w1 

#list memes
expected=$(cat ../integration/list_meme_response)
generated_output=$(echo -e "GET /meme/list HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 )

echo -e "\n\nINTEGRATION: running create and list meme test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "${expected}"
    echo "${generated_output}"
    echo -e "Create and List Meme Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Create and List Test Passed\n"
fi

#search memes
expected=$(cat ../integration/search_meme_response)
generated_output=$(echo -e "GET /meme/list?query=differ HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 )

echo -e "\n\nINTEGRATION: running search meme test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "${expected}"
    echo "${generated_output}"
    echo -e "Search Meme Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Search Meme Test Passed\n"
fi

#view meme
expected=$(cat ../integration/view_meme_response)
generated_output=$(echo -e "GET /meme/view?id=2 HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 )

echo -e "\n\nINTEGRATION: running view meme test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "${expected}"
    echo "${generated_output}"
    echo -e "View Meme Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "View Meme Test Passed\n"
fi

#edit a meme
echo -e "GET /meme/create HTTP/1.1\r\n\r\nimage=test.jpg&top=new+text&bottom=wowzer+dood&update=1" | nc localhost 8080 -w1

#view the edited meme
expected=$(cat ../integration/edit_meme_response)
generated_output=$(echo -e "GET /meme/view?id=1 HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1)

echo -e "\n\nINTEGRATION: running edit meme test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "${expected}"
    echo "${generated_output}"
    echo -e "Edit Meme Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Edit Meme Test Passed\n"
fi

#delete a meme
echo -e "GET /meme/delete?id=2 HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1

#list the remaining memes
expected=$(cat ../integration/list_after_delete_meme_response)
generated_output=$(echo -e "GET /meme/list HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 )

echo -e "\n\nINTEGRATION: running delete and list meme test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo "${expected}"
    echo "${generated_output}"
    echo -e "Delete and List Meme Test Failed\n"
    kill -s SIGINT $id
    exit 1
else
    echo -e "Delete and List Meme Test Passed\n"
fi

echo "All integration Tests Passed"
echo "======================================="
kill -s SIGINT $id