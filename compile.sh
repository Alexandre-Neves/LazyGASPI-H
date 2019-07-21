if [ ! -d "${PWD}/bin" ]; then mkdir "${PWD}/bin"; fi
g++ -pthread -Iinclude -o bin/test.o src/*.cpp -Llib64 -lGPI2
ssh alexandrepc "if [ ! -d \"${PWD}/bin\" ]; then mkdir -p \"${PWD}/bin\"; fi; exit"
scp bin/test.o alexandrepc:${PWD}/bin/test.o

