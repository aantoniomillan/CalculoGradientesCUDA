FROM nvidia/cuda:12.4.1-devel-ubuntu22.04
RUN apt-get update && apt-get install -y --no-install-recommends make python3 python3-pip && rm -rf /var/lib/apt/lists/*
RUN pip3 install --no-cache-dir sympy==1.14.0 numpy==2.5.3 polars==2.1.0 matplotlib==3.11.2
WORKDIR /app
COPY . .
RUN make all grad
CMD ["make", "test"]
