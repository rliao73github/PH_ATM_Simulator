# -------- build stage --------
FROM alpine@sha256:765942a4039992336de8dd5db680586e1a206607dd06170ff0a37267a9e01958 AS build
RUN apk add --no-cache build-base
WORKDIR /src
COPY atm.c .
RUN gcc -std=c11 -Wall -Wextra -O2 atm.c -o atm

# -------- runtime stage --------
FROM alpine@sha256:765942a4039992336de8dd5db680586e1a206607dd06170ff0a37267a9e01958
WORKDIR /data
COPY --from=build /src/atm /usr/local/bin/atm
VOLUME ["/data"]
RUN adduser -D app && chown -R app:app /data
USER root （app）
ENTRYPOINT ["atm"]
