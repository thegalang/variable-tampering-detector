# add environment variable $DynamoRIO_HOME
mkdir build
cd build
cmake -DDynamoRIO_DIR=$DynamoRIO_HOME/cmake ../src
make