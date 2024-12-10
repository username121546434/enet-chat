@echo off
g++ Client/main.cpp -Iext/include -Lext/lib -lenet64 -lws2_32 -lwinmm -static -o build/client.exe
g++ Server/main.cpp -Iext/include -Lext/lib -lenet64 -lws2_32 -lwinmm -static -o build/server.exe
