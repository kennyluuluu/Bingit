#!/bin/bash
echo "======================================="
echo "Integration proxy tests start"

# start webserver
./bin/server ../configs/8080_config & 
idBack=$!
./bin/server ../configs/8880_config & 
idFront=$!

sleep 1

# Proxy request test
expected=$'TESTTESTTEST\nTESTTESTTEST'
generated_output=$(curl -sS localhost:8880/proxy/static/test.txt)

echo -e "\n\nINTEGRATION: running Proxy request test\n"
if [ "$expected" != "$generated_output" ]; 
then
    echo -e "Proxy Request Test Failed\n"
    kill -s SIGINT $idBack $idFront 
    exit 1
else
    echo -e "Proxy Request Test Passed\n"
fi

echo "Integration Proxy Tests Passed"
echo "======================================="
kill -s SIGINT $idBack $idFront