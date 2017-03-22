CC = g++ -std=c++11
CC_FLAGS = -w -g
LIBS = -L/home/ubuntu/cproj/WSS_Server/WSS_Server/ -lwss_server -L/home/ubuntu/cproj/Boost_WS_Server/Boost_WS_Server/ -lws_server -L/home/ubuntu/cproj/HTMLHandle/HTMLHandle/ -lhtml_handle -L/home/ubuntu/cproj/AsyncServerParent/AsyncServerParent/ -lasync_server -lodbc -lboost_system -lboost_serialization -lboost_thread -lprotobuf -lcryptopp -lssl -lcrypto -L/home/ubuntu/aws-sdk-cpp/build/aws-cpp-sdk-email/ -laws-cpp-sdk-email -L/home/ubuntu/aws-sdk-cpp/build/aws-cpp-sdk-core/ -laws-cpp-sdk-core -lcurl -pthread
INCLUDES = -I/home/ubuntu/cproj/AsyncServerParent/AsyncServerParent/ -I/home/ubuntu/cproj/HTMLHandle/HTMLHandle/ -I/home/ubuntu/cproj/Boost_WS_Server/Boost_WS_Server/ -I/home/ubuntu/cproj/WSS_Server/WSS_Server/ -I/home/ubuntu/aws-sdk-cpp/aws-cpp-sdk-core/include/ -I/home/ubuntu/aws-sdk-cpp/aws-cpp-sdk-email/include/ -I/home/ubuntu/aws-sdk-cpp/aws-cpp-sdk-s3/include/

EXEC = run
SOURCES = $(wildcard *.cpp)
PROTO_SOURCES = $(shell find . -name *.pb.cc)
OBJECTS = $(SOURCES:.cpp=.o)
PROTO_OBJECTS = $(PROTO_SOURCES:.pb.cc=.o)

# Main target                                                                                                                                                                                               
$(EXEC): $(OBJECTS) $(PROTO_OBJECTS)
	$(CC) $(OBJECTS) $(PROTO_OBJECTS) -o $(EXEC) $(LIBS)

# To obtain object files                                                                                                                                                                                    
%.o: %.cpp
	$(CC) -c $(INCLUDES) $(CC_FLAGS) $< -o $@
%.o: %.pb.cc
	$(CC) -c $(INCLUDES) $(CC_FLAGS) $< -o $@

# To remove generated files                                                                                                                                                                                 
clean:
	rm -f $(EXEC) $(OBJECTS) $(PROTO_OBJECTS)
