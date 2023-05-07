FROM 32bit/ubuntu:16.04

WORKDIR /home/app

# Install required packages
COPY benchmark_sysbench.sh .
RUN apt-get -y update && apt-get -y install sysbench
RUN sh benchmark_sysbench.sh | tee docker_benchmarking.txt