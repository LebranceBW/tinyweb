FROM centos:latest
# FROM alpine:latest
WORKDIR /usr/app
COPY ./target/ /usr/app/
RUN chmod +x tinyweb
ENTRYPOINT ["./tinyweb", "0.0.0.0", "80"]
EXPOSE 80
