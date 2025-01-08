#! /bin/sh

./make.sh
./serverDriver 9099 > server_Out.txt &
serverPID=$!


#CLIENT 1 TEST
./client1 > client1_Out.txt
if diff client1_Out.txt client1_GT.txt > /dev/null; then
        echo "Client1 output matches"
else
        echo "CLIENT1 OUTPUT DIFFER"
        diff client1_Out.txt client1_GT.txt
fi


#CLIENT 2 TEST
./client2 > client2_Out.txt
if diff client2_Out.txt client2_GT.txt > /dev/null; then
        echo "Client2 output matches"
else
        echo "CLIENT2 OUTPUT DIFFER"
        diff client2_Out.txt client2_GT.txt
fi


#CLIENT 3 TEST
./client3 > client3_Out.txt
if diff client3_Out.txt client3_GT.txt > /dev/null; then
        echo "Client3 output matches"
else
        echo "CLIENT3 OUTPUT DIFFER"
        diff client3_Out.txt client3_GT.txt
fi


#CLIENT 4 TEST
./client4 > client4_Out.txt
if diff client4_Out.txt client4_GT.txt > /dev/null; then
        echo "Client4 output matches"
else
        echo "CLIENT4 OUTPUT DIFFER"
        diff client4_Out.txt client4_GT.txt
fi


#CLIENT 5 TEST
./client5 > client5_Out.txt
if diff client5_Out.txt client5_GT.txt > /dev/null; then
        echo "Client5 output matches"
else
        echo "CLIENT5 OUTPUT DIFFER"
        diff client5_Out.txt client5_GT.txt
fi

sleep 2;


#CLIENT 6 TEST
./client6 > client6_Out.txt
if diff client6_Out.txt client6_GT.txt > /dev/null; then
        echo "Client6 output matches"
else
        echo "CLIENT6 OUTPUT DIFFER"
        diff client6_Out.txt client6_GT.txt
fi


#CLIENT 7 TEST
./client7 > image1_Out.jpg
if diff image1_Out.jpg image1_GT.jpg > /dev/null; then
        echo "Client7 output matches"
else
        echo "CLIENT7 OUTPUT DIFFER"
        diff image1_Out.jpg image1_GT.jpg
fi


#CLIENT 8 TEST
./client8 > image3_Out.png
if diff image3_Out.png image3_GT.png > /dev/null; then
        echo "Client8 output matches"
else
        echo "CLIENT8 OUTPUT DIFFER"
        diff image3_Out.png image3_GT.png
fi


#CLIENT 9 TEST
./client9 > client9_Out.txt
if diff client9_Out.txt client9_GT.txt > /dev/null; then
        echo "Client9 output matches"
else
        echo "CLIENT9 OUTPUT DIFFER"
        diff client9_Out.txt client9_GT.txt
fi


#CLIENT 10 TEST
./client10 > client10_Out.txt
if diff client10_Out.txt client10_GT.txt > /dev/null; then
        echo "Client10 output matches"
else
        echo "CLIENT10 OUTPUT DIFFER"
        diff client10_Out.txt client10_GT.txt
fi


#CLIENT 11 TEST
./client11 > client11_Out.txt
if diff client11_Out.txt client11_GT.txt > /dev/null; then
        echo "Client11 output matches"
else
        echo "CLIENT11 OUTPUT DIFFER"
        diff client11_Out.txt client11_GT.txt
fi


#CLIENT 12 TEST
./client12 > client12_Out.txt
if diff client12_Out.txt client12_GT.txt > /dev/null; then
        echo "Client12 output matches"
else
        echo "CLIENT12 OUTPUT DIFFER"
        diff client12_Out.txt client12_GT.txt
fi


#CLIENT 13 TEST
./client13 > image2_Out.jpg
if diff image2_Out.jpg image2_GT.jpg > /dev/null; then
        echo "Client13 output matches"
else
        echo "CLIENT13 OUTPUT DIFFER"
        diff image2_Out.jpg image2_GT.jpg
fi


#CLIENT 14 TEST
./client14 > client14_Out.txt
if diff client14_Out.txt client14_GT.txt > /dev/null; then
        echo "Client14 output matches"
else
        echo "CLIENT14 OUTPUT DIFFER"
        diff client14_Out.txt client14_GT.txt
fi


kill $serverPID