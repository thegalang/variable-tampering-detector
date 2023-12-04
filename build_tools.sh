# add environment variable $DynamoRIO_HOME
if [ -d "./build" ]; then
	rm -r build
fi

mkdir build
cd build
cmake -DDynamoRIO_DIR=$DynamoRIO_HOME/cmake ../src
make