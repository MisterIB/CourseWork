FROM gcc:14.2.0 as build

ADD server.cpp /app/

WORKDIR /app/build

RUN g++ --std=c++23 /app/server.cpp -o server

FROM ubuntu:24.10

RUN groupadd -r sampleUser && useradd -r -g sampleUser sampleUser
USER sampleUser

WORKDIR /app

COPY --from=build /app/build/server /app

ENTRYPOINT ["./server"]
