FROM alpine:latest
RUN apk add --no-cache g++ musl-dev
COPY atm5.cpp /app/atm5.cpp
RUN g++ /app/atm5.cpp -o /app/atm5
CMD ["/app/atm5"]