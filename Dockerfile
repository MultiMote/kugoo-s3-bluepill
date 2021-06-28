FROM debian:10-slim

RUN apt-get update && apt-get install cmake make python3 gcc-arm-none-eabi dos2unix -y

COPY . /code/

WORKDIR /code

# Fix CRLF line endings
RUN mkdir /build && mkdir /dist \
&& find libopencm3 \( -name  '*.py' -o  -iname 'Makefile' -o -iname 'irq2nvic_h' \) -exec dos2unix {} \;

CMD cd /code/libopencm3 && make TARGETS=stm32/f1 \
&& cd /build \
&& cmake -G "Unix Makefiles" -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=/dist /code \
&& cmake --build .
