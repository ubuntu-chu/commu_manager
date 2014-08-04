#!/bin/sh

echo ""
echo "cd tinyxml/ to make"
echo ""
cd tinyxml/
make; make install
cd ../


echo ""
echo "cd parse/ to make"
echo ""
cd parse/
make; make install
cd ../

echo ""
echo "cd utils/ to make"
echo ""
cd utils
make; make install
cd ..

echo ""
echo "cd reactor/muduo/base/ to make"
echo ""
cd reactor/muduo/base
make; make install
cd ../../..

echo ""
echo "cd reactor/muduo/net/ to make"
echo ""
cd reactor/muduo/net
make; make install
cd ../../..

echo ""
echo "cd zygote/ to make"
echo ""
cd zygote
make; make install
cd ..

echo ""
echo "cd commout/ to make"
echo ""
cd commout
make; make install
cd ..


