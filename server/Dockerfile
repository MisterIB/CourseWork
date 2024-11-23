#FROM gcc:14.2.0 as build

#ADD server.cpp /app/
#ADD /include/pqxx /usr/include/pqxx

#WORKDIR /app/build

#RUN apt update; \
 #   apt install -y postgresql; \
 #   apt install libpqxx-7.9; \
 #   apt install -y libpqxx-dev; \
  #  ldconfig;

#RUN g++ --std=c++23 /app/server.cpp -o server -lpqxx -lpq

FROM debian:sid

ADD server /app/

ARG UID
ARG GID
ENV UID=${UID}
ENV GID=${GID}

RUN apt update; \
    apt install -y postgresql; \
    apt install -y libpqxx-7.9; \
    apt install -y libpqxx-dev; \
    ldconfig;

RUN groupadd --gid ${GID} tester && useradd --uid ${UID} -g tester tester
USER tester

WORKDIR /app

#COPY --from=build /app/build/server /app

ENTRYPOINT ["./server"]
