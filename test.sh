make clean
make all
echo j | make run > output.json
sed -i -e "1,4d" output.json
sed -i -e "s/\[d\]ebug \| output \[j\]son \| \[e\]xit: //g" output.json
